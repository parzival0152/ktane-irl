#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <pico/i2c_slave.h>
#include <pico/rand.h>
#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <pico/rand.h>

#include "../ktane-globals/def.h"
#include "button.h"
#include "display.h"

uint8_t data = 0;

static ButtonColors button_color;
static ButtonLables button_lable;
static uint8_t battery_count;
static uint8_t lit_frk = 0;
static uint8_t lit_car = 0;

static bool fail_flag = false;
static bool lose_flag = false;
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
    if (events & GPIO_IRQ_LEVEL_HIGH) printf("Level High ");
    if (events & GPIO_IRQ_LEVEL_LOW) printf("Level Low ");
    printf("\n");
}

void button_module_init() {
    button_color = (ButtonColors)(get_rand_64() % BTN_COLOR_NUM);
    button_lable = (ButtonLables)(get_rand_64() % LABLES_NUM);
    battery_count = (get_rand_64() % (MAX_BATTARYIES + 1)); // [0, 10]

    stdio_init_all(); // initalize stdio for printf
    
    /**************************
        GPIO init block
    **************************/
    // LED PIN init
    gpio_init(BUTTON_RED_PIN);
    gpio_set_dir(BUTTON_RED_PIN, GPIO_OUT);	
    gpio_init(BUTTON_GREEN_PIN);
    gpio_set_dir(BUTTON_GREEN_PIN, GPIO_OUT);	
    gpio_init(BUTTON_BLUE_PIN);
    gpio_set_dir(BUTTON_BLUE_PIN, GPIO_OUT);	

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
}

int main() {
    button_module_init();

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

    sleep_ms(1000);

    // while(state != SUCCEEDED && !lose_flag) {
    // 	// Main Loop
    // 	bool pin = gpio_get(BUTTON);
    // 	printf(pin ? "Button is High\n" : "Button is low\n");
    // 	sleep_ms(500);
    // }

    // if(state == SUCCEEDED) { // If there was a success, turn the LED GREEN and halt.
    // 	gpio_put(STATUS_GREEN_PIN, 1);
    // 	printf("Waiting forever\n");
    // 	while(true) sleep_ms(0);
    // }

    // if(lose_flag) {
    // 	gpio_put(STATUS_RED_PIN, 1);
    // 	while(true) sleep_ms(0);
    // }

    printf(
        "Status:\n"
        "\tbutton_color: %s\n"
        "\tbutton_lable: %d, %s\n"
        "\tbattery_count: %d\n",
        button_color == RED     ? "RED" :
        button_color == WHITE   ? "WHITE" :
        button_color == BLUE    ? "BLUE" :
        button_color == YELLOW  ? "YELLOW" : "BAD VALUE",
        button_lable,
        BUTTON_LABLES[button_lable],
        battery_count);


    uint16_t centered_x = (SSD1306_WIDTH - strlen(BUTTON_LABLES[button_lable]) * FONT_CHAR_WIDTH) / 2;
    WriteString(buf, centered_x, 0, BUTTON_LABLES[button_lable]);
    render(buf, &frame_area);

    switch (button_color)
    {
    case BLUE:
        gpio_put(BUTTON_BLUE_PIN, 1);
        break;
    case WHITE:
        gpio_put(BUTTON_BLUE_PIN, 1);
    case YELLOW:
        gpio_put(BUTTON_GREEN_PIN, 1);
    case RED:
        gpio_put(BUTTON_RED_PIN, 1);
    }

    while (true) {
        sleep_ms(0);
    }
}
