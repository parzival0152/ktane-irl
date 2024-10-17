#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/i2c_slave.h"
#include "pico/rand.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "../ktane-globals/def.h"

#define MODULE_I2C i2c0
#define PERIPHERAL_I2C i2c1

#define BAR_DELAY 500

const uint8_t MASTER_SDA = 14;
const uint8_t MASTER_SCL = 15;
const uint8_t SLAVE_SDA = 16;
const uint8_t SLAVE_SCL = 17;

const uint8_t BUTTON = 21;

const uint8_t BUTTON_RED = 12;
const uint8_t BUTTON_GREEN = 11;
const uint8_t BUTTON_BLUE = 10;

const uint8_t BAR_RED_PIN = 12;
const uint8_t BAR_GREEN_PIN = 11;
const uint8_t BAR_BLUE_PIN = 10;

const uint8_t STATUS_RED = 12;
const uint8_t STATUS_GREEN = 11;
const uint8_t STATUS_BLUE = 10;

uint8_t data = 0;

static uint8_t battery_count = 0;
static uint8_t lit_frk = 0;
static uint8_t lit_car = 0;

enum button_colors {
	RED,
	WHITE,
	BLUE,
	YELLOW
} button_color;

enum bar_colors {
	BAR_RED,
	BAR_WHITE,
	BAR_BLUE,
	BAR_YELLOW
} bar_color;

enum button_states {
	IDLE,
	OTHER
} button_state;

static bool fail_flag = 0;
static bool lose_flag = 0;
static enum i2c_slave_responses state = READY_2_START;

static uint8_t recv_data = 1;

const uint I2C_SLAVE_ADDRESS = 0x19;

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
		case I2C_SLAVE_RECEIVE: // Master is writing to the module
			data = i2c_read_byte_raw(i2c);
			if(recv_data) {
				battery_count = data & BATTARY_COUNT_MASK;
				lit_frk = data & LIT_FRK_INDICATOR;
				lit_car = data & LIT_CAR_INDICATOR;
				recv_data--;
			}
			else {
				switch (data) {
					case INIT_INFO:
						recv_data = INIT_FRAME;
						break;
					case START:
						state = (state == READY_2_START) ? RUNNING : state;
						break;
					case GAME_LOSS:
						lose_flag = 1;
						break;
					default:
						break;
				}
			}
			break;
		case I2C_SLAVE_REQUEST: // Master is reading from the module
			i2c_write_byte_raw(i2c, state);
			state = state == FAILED ? RUNNING : state; // reset state to not submit multiple fails
			break;
		default:
			break;
    }
}

static void gpio_handler(uint gpio, uint32_t events) {
	if(gpio != BUTTON){
		printf("Bro What\n");
		return;
	}
	if (events & GPIO_IRQ_EDGE_RISE) {
		return;
	}
	if (events & GPIO_IRQ_EDGE_FALL) printf("Edge fall ");
	if (events & GPIO_IRQ_LEVEL_LOW) printf("Level Low ");
	if (events & GPIO_IRQ_LEVEL_HIGH) printf("Level High ");
	printf("\n");

}

int main() {
	stdio_init_all(); // initalize stdio for printf
	
	/**************************
		GPIO init block
	**************************/
	// LED PIN init
	gpio_init(BUTTON_RED);
	gpio_set_dir(BUTTON_RED, GPIO_OUT);	
	gpio_init(BUTTON_GREEN);
	gpio_set_dir(BUTTON_GREEN, GPIO_OUT);	
	gpio_init(BUTTON_BLUE);
	gpio_set_dir(BUTTON_BLUE, GPIO_OUT);	

	// Setting up the i2c slave
	gpio_init(SLAVE_SDA);
	gpio_init(SLAVE_SCL);
	gpio_set_function(SLAVE_SDA, GPIO_FUNC_I2C);
	gpio_set_function(SLAVE_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(SLAVE_SDA);
	gpio_pull_up(SLAVE_SCL);

    i2c_init(MODULE_I2C, I2C_BAUDRATE);
    i2c_slave_init(MODULE_I2C, I2C_SLAVE_ADDRESS, &i2c_slave_handler);

	// Setting up the i2c master
	gpio_init(MASTER_SDA);
	gpio_init(MASTER_SCL);
	gpio_set_function(MASTER_SDA, GPIO_FUNC_I2C);
	gpio_set_function(MASTER_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(MASTER_SDA);
	gpio_pull_up(MASTER_SCL);

    i2c_init(PERIPHERAL_I2C, I2C_BAUDRATE);

	// Setting up the IRQ for the buttons to switch freq and to transmit
	gpio_pull_down(BUTTON);
	gpio_init(BUTTON);
	gpio_set_dir(BUTTON, GPIO_IN);

    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_RISE  | GPIO_IRQ_EDGE_FALL, true, &gpio_handler);

	while(state != SUCCEEDED && !lose_flag){
		// Main Loop
		bool pin = gpio_get(BUTTON);
		printf(pin ? "Button is High\n" : "Button is low\n");
		sleep_ms(500);
	}
	//if(state == SUCCEEDED) { // If there was a success, turn the LED GREEN and halt.
	//	gpio_put(GREEN, 1);
	//	printf("Waiting forever\n");
	//	while(1) {
	//		sleep_ms(50);
	//	}
	//}
	//if(lose_flag) {
	//	gpio_put(RED, 1);
	//	while(1) {
	//		sleep_ms(50);
	//	}
	//
	//}
}
