#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/i2c_slave.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "resolver.c"
#include "../ktane-globals/def.h"

#define MODULE_I2C i2c0
#define SAMPLE_COUNT 10

const uint8_t SLAVE_SDA = 16;
const uint8_t SLAVE_SCL = 17;

const uint8_t I2C_SLAVE_ADDRESS = 0x17;

const uint8_t ADC_IN = 26;
const uint8_t CTRL_0 = 5;
const uint8_t CTRL_1 = 6;
const uint8_t CTRL_2 = 7;

const uint8_t STATUS_RED 	= 8;
const uint8_t STATUS_GREEN 	= 9;
const uint8_t STATUS_BLUE 	= 10;

uint8_t data = 0;
static uint8_t recv_data = 0;

static enum wire_color wires[6] = {NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR};
static enum wire_color solve_matrix[6] = {NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR, NO_COLOR};
static uint8_t solve_matrix_length = 0;
static uint8_t correct_index = -1;
static bool last_serial_odd;
static bool config_recv_flag = false;

static enum i2c_slave_responses state = SETTING_UP;
static bool lose_flag = 0;

uint8_t convert_index_to_real(uint8_t current_index) {
	for(int i = 0; i < 6;  i++) {
		if(wires[i] != NO_COLOR){
			if(current_index == 0) return i;
			else current_index--;
		}
	}
}

static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
		case I2C_SLAVE_RECEIVE: // Master is writing to the module
			data = i2c_read_byte_raw(i2c);
			if(recv_data) {
				if(recv_data == INIT_FRAME) { // if this is the first byte of the frame
					last_serial_odd = data & SERIAL_NUMBER_ODD;	
					config_recv_flag = true;
				}
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
void switch_adc_to_index(uint index) {
	// switch the ADC multiplexer to the given index (lower 3 bits of the index)
	printf("Switching the ADC to index %d\n", index);
	gpio_put(CTRL_0, (index & 1));
	gpio_put(CTRL_1, (index & 2));
	gpio_put(CTRL_2, (index & 4));
}

uint16_t read_from_index(uint index) {
	switch_adc_to_index(index);
	sleep_ms(50);
	uint32_t read_buffer = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++) {
		read_buffer += adc_read();
		sleep_ms(15);
	}
	read_buffer /= SAMPLE_COUNT;
	return read_buffer;
}

enum wire_color adc_to_color(uint16_t data) {
	// Function to convert from the ADC measurement to the color of the wire,
	if(data <= NO_COLOR) return NO_COLOR;
	if(data <= WHITE) return WHITE;
	if(data <= BLUE) return BLUE;
	if(data <= YELLOW) return YELLOW;
	if(data <= RED) return RED;
	return BLACK;
}

enum wire_color read_color(uint index) {
	return adc_to_color(read_from_index(index));
}

void print_color(uint16_t data) {
	printf("The color of the wire is: ");
	switch (adc_to_color(data)){
		case NO_COLOR:
			printf("NO_COLOR\n");
			break;
		case WHITE:
			printf("WHITE\n");
			break;
		case BLUE:
			printf("BLUE\n");
			break;
		case YELLOW:
			printf("YELLOW\n");
			break;
		case RED:
			printf("RED\n");
			break;
		case BLACK:
			printf("BLACK\n");
			break;
	}
}

void setup_wires () {
	for(uint i = 0; i < 6; i++) {
		enum wire_color color = read_color(1);
		wires[i] = color;
		if(color != NO_COLOR){
			solve_matrix[solve_matrix_length++] = color;
		}
		sleep_ms(50);
	}
	state = READY_2_START;
}

int main() {
	stdio_init_all(); // initalize stdio for printf
	
	/**************************
		GPIO init block
	**************************/
	// LED PIN init
	gpio_init(STATUS_RED);
	gpio_set_dir(STATUS_RED, GPIO_OUT);	
	gpio_init(STATUS_GREEN);
	gpio_set_dir(STATUS_GREEN, GPIO_OUT);	

	// ADC control Pin init
	gpio_init(CTRL_0);
	gpio_init(CTRL_1);
	gpio_init(CTRL_2);
	gpio_set_dir(CTRL_0, GPIO_OUT);	
	gpio_set_dir(CTRL_1, GPIO_OUT);	
	gpio_set_dir(CTRL_2, GPIO_OUT);	
	
	// ADC HW init
	adc_init();
	adc_gpio_init(ADC_IN);
	adc_select_input(0);
	
	// I2C slave init
	gpio_init(SLAVE_SDA);
	gpio_init(SLAVE_SCL);
	gpio_set_function(SLAVE_SDA, GPIO_FUNC_I2C);
	gpio_set_function(SLAVE_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(SLAVE_SDA);
	gpio_pull_up(SLAVE_SCL);

    i2c_init(MODULE_I2C, I2C_BAUDRATE);
    i2c_slave_init(MODULE_I2C, I2C_SLAVE_ADDRESS, &i2c_slave_handler);

	while(1) {
		for(int i = 0; i < 6; i++){
			print_color(read_color(i));
			sleep_ms(500);
		}
	}

	while(!config_recv_flag){ // Wait till the config data has been given
		sleep_ms(50);
	}

	// Read the connected wires and calculate correct solution
	setup_wires();
	if(solve_matrix_length < 3) {
		gpio_put(STATUS_RED, 1);
		while(1) {
			printf("Config Error: Too few wires attached\n");
			sleep_ms(250);
		}
	}
	correct_index = convert_index_to_real(get_correct_index(solve_matrix, solve_matrix_length, last_serial_odd));
	while(state != SUCCEEDED && !lose_flag){
		//TODO: module gameplay procedure
	}
	if(state == SUCCEEDED) { // If there was a success, turn the LED GREEN and halt.
		gpio_put(STATUS_GREEN, 1);
		printf("Waiting forever\n");
		while(1) {
			sleep_ms(50);
		}
	}
	if(lose_flag) {
		gpio_put(STATUS_RED, 1);
		while(1) {
			sleep_ms(50);
		}
	}
}
