#ifndef BUTTON_H
#define BUTTON_H

#define MODULE_I2C i2c0
#define PERIPHERAL_I2C i2c1

#define BAR_DELAY 500

// i2c pins
const uint8_t MASTER_SDA = 14;
const uint8_t MASTER_SCL = 15;
const uint8_t SLAVE_SDA = 16;
const uint8_t SLAVE_SCL = 17;

// button color pins
const uint8_t BUTTON_RED = 18;
const uint8_t BUTTON_GREEN = 19;
const uint8_t BUTTON_BLUE = 20;

// button pin
const uint8_t BUTTON = 21;

// status pins
const uint8_t STATUS_RED = 24;
const uint8_t STATUS_GREEN = 25;
const uint8_t STATUS_BLUE = 26;

// bar pins
const uint8_t BAR_RED_PIN = 27;
const uint8_t BAR_GREEN_PIN = 28;
const uint8_t BAR_BLUE_PIN = 29;

enum ButtonColors {
	RED,
	WHITE,
	BLUE,
	YELLOW
};

enum BarColors {
	BAR_RED,
	BAR_WHITE,
	BAR_BLUE,
	BAR_YELLOW
};

enum ButtonStates {
	IDLE,
	OTHER
};

enum ButtonLables {
	PRESS = 0,
    ABORT = 1,
    DETONATE = 2,
    HOLD = 3
};

const char* const BUTTON_LABLES[] = {
	"PRESS"
    "ABORT",
    "DETONATE",
    "HOLD"
};

#endif