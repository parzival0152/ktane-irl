#include <stdio.h>
#include <PicoTM1637.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/rand.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "../ktane-globals/def.h"

/****************************
 * Defintion block
****************************/
#define MODULE_I2C i2c0
#define SCREEN_I2C i2c1

#define SCREEN_SDA  14
#define SCREEN_SCL  15
#define MODULE_SDA  16
#define MODULE_SCL  17

#define CLK_PIN 20
#define DIO_PIN 19

#define MAX_MODULE_COUNT 16
#define BUZZER_TIMER_PITCH 50000

const uint8_t LED_PIN = 9;
const uint8_t FIRST_FAIL = 23;
const uint8_t SECOND_FAIL = 22;
const uint8_t START_BUTTON = 4;
const uint8_t BUZZER_PIN = 21;

const int BUZZER_ON_DELAY 	= -200000;
const int BUZZER_OFF_DELAY 	= -800000;
const uint8_t DISPLAY_BRIGHTNESS = 4;
const uint32_t START_DELAY = 2000;

const uint8_t MIN_MODULE_COUNT = 1;

static uint8_t addresses[MAX_MODULE_COUNT] = {0};
static uint8_t module_count = 0;

static volatile bool done = 0;
static bool start = 0;
static volatile bool game_finished = 0;

static uint8_t init_data;
static uint8_t rxdata = 0;

static int16_t game_timer;
static struct repeating_timer game_timer_timer;
static uint8_t  fails = 0;

static char beep_edge = false;

volatile enum master_states {
	NOT_READY,
	READY_2_GO,
	GOING,
	DONE
} master_state = NOT_READY;


void gpio_callback(uint gpio, uint32_t events) {
	printf("Interupt called from pin %d\n");
	if((events & GPIO_IRQ_EDGE_RISE) && (master_state == READY_2_GO)) {
		master_state = GOING;
		printf("setting the master state to be GOING\n");
	}
	printf("master state is: %d\n", master_state);
}

int64_t beep_callback(alarm_id_t id, void *user_data) {

	if(game_timer == 0 || game_finished) {
		gpio_put(BUZZER_PIN, 0);
		return 0;
	}
	
	if(beep_edge){
		// clock_stop(CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS);
		gpio_put(BUZZER_PIN, 0);
	}
	else{
    	// clock_gpio_init(BUZZER_PIN, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, BUZZER_TIMER_PITCH);
		gpio_put(BUZZER_PIN, 1);
	}
	beep_edge = !beep_edge;
	return beep_edge ? BUZZER_ON_DELAY : BUZZER_OFF_DELAY;
}

bool clock_countdown_callback(struct repeating_timer *t) {
	bool update = (game_timer >= 0 && !game_finished);
	
	if(update) {
    	TM1637_display_both(game_timer/60, game_timer % 60, true);
		game_timer--;
	}
	return update;
}

void setup_master() {

	/**************************
		GPIO init block
	**************************/
	// LED PIN init
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);	
	gpio_init(FIRST_FAIL);
	gpio_set_dir(FIRST_FAIL, GPIO_OUT);	
	gpio_init(SECOND_FAIL);
	gpio_set_dir(SECOND_FAIL, GPIO_OUT);	
	
	gpio_pull_down(START_BUTTON);

	gpio_init(BUZZER_PIN);
	gpio_set_dir(BUZZER_PIN, GPIO_OUT);

	// Initialize all the i2c busses
	gpio_init(MODULE_SDA);
	gpio_init(MODULE_SCL);
	gpio_set_function(MODULE_SDA, GPIO_FUNC_I2C);
	gpio_set_function(MODULE_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(MODULE_SDA);
	gpio_pull_up(MODULE_SCL);

	i2c_init(MODULE_I2C, I2C_BAUDRATE);	
	
	gpio_init(SCREEN_SDA);
	gpio_init(SCREEN_SCL);
	gpio_set_function(SCREEN_SDA, GPIO_FUNC_I2C);
	gpio_set_function(SCREEN_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(SCREEN_SDA);
	gpio_pull_up(SCREEN_SCL);

	i2c_init(SCREEN_I2C, I2C_BAUDRATE);

	// TODO: read some pins to configure the amount of time that we start with

	game_timer = 3 * 60; // set the amount of time for the game to be 5 mins
}

void populate_addresses() {
	int ret;
	for(uint i = 1; i < 128 && module_count < MAX_MODULE_COUNT; i++){ 
		ret = i2c_read_blocking(MODULE_I2C, i, &rxdata, 1, false);
		if(ret >= 0) {
			addresses[module_count++] = i;
			printf("Found module at address: %x\n", i);
		}
	}
}

void init_game() {
	uint8_t random_config = get_rand_32(); // randomize the config
	random_config = SERIAL_NUMBER_ODD | SERIAL_NUMEBR_VOWEL | LIT_FRK_INDICATOR;
	uint8_t txdata[] = {
		INIT_INFO,
		random_config
	}; // create the data to send

	i2c_write_blocking(MODULE_I2C, 0x00, txdata, 2, false); // send the init to all modules
	
	done = 0;
	while(!done) {
		done = 1;
		for(uint8_t i = 0; i < module_count; i++){
			i2c_read_blocking(MODULE_I2C, addresses[i], &rxdata, 1, false);
			done = (rxdata != READY_2_START) ? 0 : done;
			printf("Reading state from address: %x, it is %d\n", addresses[i], rxdata);
		}
	}
	gpio_put(LED_PIN, 1);
}

void start_game() {
	uint8_t txdata = START;
	i2c_write_blocking(MODULE_I2C, 0x00, &txdata, 1, false);
    add_repeating_timer_ms(-1000, clock_countdown_callback, NULL, &game_timer_timer);
	add_alarm_in_ms(1000, beep_callback, NULL, false);
}

void update_fails() {
	gpio_put(FIRST_FAIL, fails > 0 ? 1 : 0);
	gpio_put(SECOND_FAIL, fails > 1 ? 1 : 0);
}

void loop() {
	uint8_t txdata = MODULE_FAILED;
	done = 0;
	while(!done && fails < 3 && game_timer > 0) {
		done = 1;
		update_fails();
		for(uint8_t i = 0; i < module_count; i++){
			i2c_read_blocking(MODULE_I2C, addresses[i], &rxdata, 1, false);
			done = (rxdata != SUCCEEDED) ? 0 : done;
			if (rxdata == FAILED) {
				i2c_write_blocking(MODULE_I2C, 0x00, &txdata, 1, false);
				fails++;
				printf("Detected that a module has failed!\n");
			}
			printf("Reading state from addres: %x, it is %d\n", addresses[i], rxdata);
			sleep_ms(50);
		}
		//printf("done is: %d, the number of fails are: %d\n", done, fails);
		sleep_ms(50);
	}
	// game is done, handle it
	game_finished = true;
}

int main() {
	uint8_t txdata;
	stdio_init_all(); // initalize stdio for printf
	alarm_pool_init_default(); // Init the alarm pool
	
	setup_master(); // Do the setup for the master
	
	TM1637_init(CLK_PIN, DIO_PIN);  
    TM1637_clear(); 
    TM1637_set_brightness(DISPLAY_BRIGHTNESS);
	sleep_ms(START_DELAY);
	printf("master state is: %d\n", master_state);
	printf("Finished Waiting\n");
	populate_addresses();
	if(module_count < MIN_MODULE_COUNT){
		printf("Less then min modules counted!\n");
		gpio_put(FIRST_FAIL, 1);	
		while(1);
	}
	init_game();
	gpio_put(LED_PIN, 1);
	master_state = READY_2_GO; // wait for user to push the start button to begin the game
	gpio_set_irq_enabled_with_callback(START_BUTTON, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
 	TM1637_display_both(00,00,true);
	while(master_state == READY_2_GO);

	gpio_put(LED_PIN, 0);
	sleep_ms(1000);
	start_game();
	loop();

	gpio_put(BUZZER_PIN, 0);

	if(fails >= 3 || game_timer == 0) {
		printf("KABOOM! the bomb has exploded and you are ash\n");
		TM1637_display_word("FAIL", true);	
		txdata = GAME_LOSS;
		i2c_write_blocking(MODULE_I2C, 0x00, &txdata, 1, false);
	}
	else {
		printf("Congratulations, you have defused the bomb!\n");
		TM1637_display_word("PASS", true);	
	}

	
	printf("Game is Done!\n");
	while(1); // loop forever
}
