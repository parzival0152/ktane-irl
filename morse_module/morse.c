#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/i2c_slave.h"
#include "pico/rand.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "blinker.c"
#include "module_data.c"
#include "lcd_1602_i2c.c"
#include "../ktane-globals/def.h"

#define MODULE_I2C i2c0
#define PERIPHERAL_I2C i2c1

const uint MASTER_SDA = 14;
const uint MASTER_SCL = 15;
const uint SLAVE_SDA = 16;
const uint SLAVE_SCL = 17;

const uint FREQ_DEC = 10;
const uint FREQ_INC = 11;
const uint TX = 12;

const uint8_t RED = 7;
const uint8_t GREEN = 8;
const uint8_t SIGNAL_LED = 9;

uint8_t data = 0;

static uint8_t correct_index;
static uint8_t starting_index;
static bool fail_flag = 0;
static bool lose_flag = 0;
static enum i2c_slave_responses state = READY_2_START;

static uint8_t recv_data = 0;

const uint I2C_SLAVE_ADDRESS = 0x18;
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

static void gpio_handler(uint gpio, uint32_t events) {
	if(!(events & GPIO_IRQ_EDGE_RISE))
		return;
	switch(gpio) {
		case FREQ_DEC:
			// printf("button has been pressed for freq dec\n");
			starting_index = starting_index == 0 ? 0 : (starting_index - 1);
			break;
		case FREQ_INC:
			// printf("button has been pressed for freq inc\n");
			starting_index = starting_index == 0xf ? 0xf : (starting_index + 1);
			break;
		case TX:
			printf("TX button has been pressed\n");
			printf("sent: %x, correc: %x\n",starting_index, correct_index);
			state = (starting_index != correct_index) ? FAILED : SUCCEEDED;
			fail_flag = state == FAILED;
			break;
		default:
			printf("Bro, how did this happen\n");
	}
}

static void setup_module_data() {
	uint32_t seed_data = get_rand_32();	
	correct_index = seed_data & 0x0f;
	starting_index = (seed_data >> 4) & 0x0f;
	if(correct_index == starting_index)
		starting_index = (starting_index + 1) % 0x10;
	printf("Seed is: %x\nCorrect is: %x\nStart is: %x\n",seed_data, correct_index, starting_index);
}

void display_freq() {
	lcd_set_cursor(PERIPHERAL_I2C, SCREEN_ADDR, 0, 0);
	lcd_string(PERIPHERAL_I2C, SCREEN_ADDR, "----------------");
	lcd_set_cursor(PERIPHERAL_I2C, SCREEN_ADDR, 0, starting_index);
	lcd_string(PERIPHERAL_I2C, SCREEN_ADDR, "|");
}

static void do_blink(){
	while(1) {
		blink_word(SIGNAL_LED, words[correct_index], 3);
	}
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
	gpio_init(SIGNAL_LED);
	gpio_set_dir(SIGNAL_LED, GPIO_OUT);	
	
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
	gpio_pull_down(FREQ_DEC);
	gpio_pull_down(FREQ_INC);
	gpio_pull_down(TX);

    gpio_set_irq_enabled_with_callback(FREQ_DEC, GPIO_IRQ_EDGE_RISE, true, &gpio_handler);
    gpio_set_irq_enabled_with_callback(FREQ_INC, GPIO_IRQ_EDGE_RISE, true, &gpio_handler);
    gpio_set_irq_enabled_with_callback(TX, GPIO_IRQ_EDGE_RISE, true, &gpio_handler);

	lcd_init(PERIPHERAL_I2C, SCREEN_ADDR);
	lcd_set_cursor(PERIPHERAL_I2C, SCREEN_ADDR, 0, 0);
	lcd_string(PERIPHERAL_I2C, SCREEN_ADDR, "Hello");

	setup_module_data();
	multicore_launch_core1(do_blink);
	uint8_t current_index = starting_index;
	while(state != SUCCEEDED && !lose_flag){
		//printf("current: %x\nstarting: %x", current_index, starting_index);
		//sleep_ms(500);
		if(starting_index != current_index){ // If the freq was changed, update on the serial monitor
			print_freq(starting_index);
			current_index = starting_index;
		}
		if(fail_flag) { // If there was a FAIL, hold the LED red for a bit and then go back to normal.
			gpio_put(RED, 1);
			sleep_ms(500);
			gpio_put(RED, 0);
			fail_flag = 0;
		}
		sleep_ms(50);
	}
	if(state == SUCCEEDED) { // If there was a success, turn the LED GREEN and halt.
		gpio_put(GREEN, 1);
		printf("Waiting forever\n");
		while(1) {
			sleep_ms(50);
		}
	}
	if(lose_flag) {
		gpio_put(RED, 1);
		while(1) {
			sleep_ms(50);
		}
	
	}
}
