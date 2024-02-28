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

#define CLK_PIN 24
#define DIO_PIN 25

#define MAX_MODULE_COUNT 16
#define BUZZER_TIMER_PITCH 50000

const uint LED_PIN = 9;
const uint FIRST_FAIL = 11;
const uint SECOND_FAIL = 12;
const uint START_BUTTON = 13;
const uint BUZZER_PIN = 21;

static uint8_t addresses[MAX_MODULE_COUNT] = {0};
static uint8_t module_count = 0;

static bool done = 0;
static bool start = 0;

static uint8_t init_data;
static uint8_t rxdata = 0;

static int16_t game_timer;
static struct repeating_timer game_timer_timer;
static uint8_t  fails = 0;

static char fire = false;

static enum master_states {
	NOT_READY,
	READY_2_GO,
	GOING,
	DONE
} master_state = NOT_READY;


void gpio_callback(uint gpio, uint32_t events) {
	if((gpio == 2) && (events & GPIO_IRQ_EDGE_RISE) && (master_state == READY_2_GO)) {
		master_state = GOING;
	}
}

int64_t beep_callback(alarm_id_t id, void *user_data) {
    clock_gpio_init(BUZZER_PIN, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, fire ? 10 : BUZZER_TIMER_PITCH);
	fire = ~fire;
	return fire ? -200000 : -800000;
}

bool clock_countdown_callback(struct repeating_timer *t) {
	
    TM1637_display_both(game_timer/60, game_timer % 60, true);
	game_timer--;

	return game_timer >= 0;
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

	// TODO: read the hardcode input pin
	gpio_set_irq_enabled_with_callback(START_BUTTON, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
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

void loop() {
	uint8_t txdata = MODULE_FAILED;
	done = 0;
	while(!done && fails < 3 && game_timer > 0) {
		done = 1;
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
}

int main() {
	uint8_t txdata;
	stdio_init_all(); // initalize stdio for printf
	alarm_pool_init_default(); // Init the alarm pool
	
	setup_master(); // Do the setup for the master
	
	TM1637_init(CLK_PIN, DIO_PIN);  
    TM1637_clear(); 
    TM1637_set_brightness(4); // max value, default is 0
	
	sleep_ms(5000);
	
	printf("Finished Waiting\n");
	populate_addresses();
	if(module_count == 0){
		printf("Error: No modules detected!\n");
		gpio_put(FIRST_FAIL, 1);	
		while(1);
	}
	init_game();
	master_state = READY_2_GO; // wait for user to push the start button to begin the game
	while(master_state == READY_2_GO);
	start_game();
	loop();
	if(fails >= 3 || game_timer == 0) {
		printf("KABOOM! the bomb has exploded and you are ash\n");
		txdata = GAME_LOSS;
		i2c_write_blocking(MODULE_I2C, 0x00, &txdata, 1, false);
	}
	else {
		printf("Congratulations, you have defused the bomb!\n");
	}

	
	printf("Game is Done!\n");
	while(1); // loop forever
}
