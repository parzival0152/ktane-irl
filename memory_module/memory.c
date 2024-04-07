#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/i2c_slave.h"
#include "pico/rand.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "../ktane-globals/def.h"

#define MODULE_I2C i2c0

/***********************
	GPIO declerations
***********************/

const uint MASTER_SDA = 14;
const uint MASTER_SCL = 15;
const uint SLAVE_SDA = 16;
const uint SLAVE_SCL = 17;

const uint8_t RED = 25;
const uint8_t GREEN = 24;
const uint8_t BUTTON_1 = 4;
const uint8_t BUTTON_2 = 5;
const uint8_t BUTTON_3 = 6;
const uint8_t BUTTON_4 = 7;

const uint8_t BAR_1 = 22;
const uint8_t BAR_2 = 21;
const uint8_t BAR_3 = 20;
const uint8_t BAR_4 = 19;
const uint8_t BAR_5 = 18;

const uint8_t STORE_CLK 	= 13;
const uint8_t SERIAL_CLK 	= 14;
const uint8_t OE 			= 12;
const uint8_t DATA_IN 		= 11;
const uint8_t CLR_DSP 		= 15;

/***************************
	Operational variables
***************************/

uint8_t data = 0;

static bool fail_flag = 0;
static bool lose_flag = 0;
static enum i2c_slave_responses state = READY_2_START;

static uint8_t recv_data = 0;

const uint I2C_SLAVE_ADDRESS = 0x1A;
const uint SCREEN_ADDR = 0x27;

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
		case I2C_SLAVE_RECEIVE: // Master is writing to the module
			data = i2c_read_byte_raw(i2c);
			if(recv_data) {
				// This module dosent need the init info data so we just decrement the counter
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

static void setup_module_data() {
	// TODO: impelemt this to the correct module data
}


int main() {
	stdio_init_all(); // initalize stdio for printf
	
	/**************************
		GPIO init block
	**************************/
	// LED PIN init
	gpio_init(RED);
	gpio_set_dir(RED, GPIO_OUT);	
	gpio_init(GREEN);
	gpio_set_dir(GREEN, GPIO_OUT);	
	gpio_init(OE);
	gpio_set_dir(OE, GPIO_OUT);	
	gpio_init(DATA_IN);
	gpio_set_dir(DATA_IN, GPIO_OUT);	
	gpio_init(STORE_CLK);
	gpio_set_dir(STORE_CLK, GPIO_OUT);	
	gpio_init(SERIAL_CLK);
	gpio_set_dir(SERIAL_CLK, GPIO_OUT);	
	gpio_init(CLR_DSP);
	gpio_set_dir(CLR_DSP, GPIO_OUT);	
	
	// Setting up the i2c slave
	gpio_init(SLAVE_SDA);
	gpio_init(SLAVE_SCL);
	gpio_set_function(SLAVE_SDA, GPIO_FUNC_I2C);
	gpio_set_function(SLAVE_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(SLAVE_SDA);
	gpio_pull_up(SLAVE_SCL);

    i2c_init(MODULE_I2C, I2C_BAUDRATE);
    i2c_slave_init(MODULE_I2C, I2C_SLAVE_ADDRESS, &i2c_slave_handler);

	setup_module_data();
	
	gpio_put(CLR_DSP, 1);
	gpio_put(OE, 0);
	sleep_us(10);
	gpio_put(DATA_IN, 1);
	sleep_us(10);
	for(uint i = 0; i < 40; i++){
		gpio_put(SERIAL_CLK, 1);
		sleep_ms(25);
		gpio_put(SERIAL_CLK, 0);
		sleep_ms(25);
		gpio_put(STORE_CLK, 1);
		sleep_ms(25);
		gpio_put(STORE_CLK, 1);
		sleep_ms(25);
	}



	/**********************
		main game loop
	**********************/
	while(state != SUCCEEDED && !lose_flag){
	}
	// win state
	if(state == SUCCEEDED) { // If there was a success, turn the LED GREEN and halt.
		gpio_put(GREEN, 1);
		gpio_put(RED, 0);
		printf("Waiting forever\n");
		while(1) {
			sleep_ms(50);
		}
	}
	// lose state
	if(lose_flag) {
		gpio_put(RED, 1);
		gpio_put(GREEN, 0);
		while(1) {
			sleep_ms(50);
		}
	}
}
