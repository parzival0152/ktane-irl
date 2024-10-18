#ifndef BUTTON_H
#define BUTTON_H

#define MODULE_I2C i2c0

#define BAR_DELAY 500

// i2c pins
const uint8_t MASTER_SDA = 14;
const uint8_t MASTER_SCL = 15;
const uint8_t SLAVE_SDA = 16;
const uint8_t SLAVE_SCL = 17;

// button color pins
const uint8_t BUTTON_RED_PIN = 18;
const uint8_t BUTTON_GREEN_PIN = 19;
const uint8_t BUTTON_BLUE_PIN = 20;

// button pin
const uint8_t BUTTON = 21;

// status pins
const uint8_t STATUS_RED_PIN = 24;
const uint8_t STATUS_GREEN_PIN = 25;
const uint8_t STATUS_BLUE_PIN = 26;

// bar pins
const uint8_t BAR_RED_PIN = 27;
const uint8_t BAR_GREEN_PIN = 28;
const uint8_t BAR_BLUE_PIN = 29;

typedef enum {
	RED = 0,
	WHITE,
	BLUE,
	YELLOW,
	BTN_COLOR_NUM
} ButtonColors;

typedef enum {
	BAR_RED = 0,
	BAR_WHITE,
	BAR_BLUE,
	BAR_YELLOW,
	BAR_COLOR_NUM
} BarColors;

typedef enum {
	IDLE,
	OTHER
} ButtonStates;

typedef enum {
	PRESS = 0,
    ABORT,
    DETONATE,
    HOLD,
	LABLES_NUM
} ButtonLables;

const char* const BUTTON_LABLES[] = {
	"PRESS",
    "ABORT",
    "DETONATE",
    "HOLD"
};

#endif