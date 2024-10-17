#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/i2c_slave.h>
#include <pico/rand.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include "../ktane-globals/def.h"
#include "button.h"
#include "display.h"

uint8_t data = 0;

static uint8_t battery_count = 0;
static uint8_t lit_frk = 0;
static uint8_t lit_car = 0;

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

    SSD1306_init();
	//while(state != SUCCEEDED && !lose_flag){
	//	// Main Loop
	//	bool pin = gpio_get(BUTTON);
	//	printf(pin ? "Button is High\n" : "Button is low\n");
	//	sleep_ms(500);
	//}

	//if(state == SUCCEEDED) { // If there was a success, turn the LED GREEN and halt.
	//	gpio_put(GREEN, 1);
	//	printf("Waiting forever\n");
	//	while(1) {
	//	}
	

	//}
	//if(lose_flag) {
	//	gpio_put(RED, 1);
	//	while(1) {
	//		sleep_ms(50);
	//	}
	//
	//}
	render_area frame_area = {
        start_col: 0,
        end_col : SSD1306_WIDTH - 1,
        start_page : 0,
        end_page : SSD1306_NUM_PAGES - 1
    };

    calc_render_area_buflen(&frame_area);

    // zero the entire display
    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);


	char* text = "DETONATE";
	WriteString(buf, 0, 0, text);
	render(buf, &frame_area);

	while (true) {
		sleep_ms(50);
	}
}
