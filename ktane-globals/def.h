#ifndef KTANE_GLOBAL_DEF_H
#define KTANE_GLOBAL_DEF_H

#include <stdint.h>

enum i2c_slave_responses {
	SETTING_UP,
	READY_2_START,
	RUNNING,
	FAILED,
	SUCCEEDED
};

enum i2c_master_commands {
	INIT_INFO,
	START,
	STATUS_CHECK,
	MODULE_FAILED,
	GAME_LOSS
};

#define I2C_BAUDRATE 100 * 1000 // 100 kHz

#define INIT_FRAME 1 // The amount of bytes that follow the INIT_INFO command

#define SERIAL_NUMBER_ODD  0x10
#define SERIAL_NUMBER_VOWEL 0x20
#define LIT_CAR_INDICATOR 0x40
#define LIT_FRK_INDICATOR 0x80
#define BATTERY_COUNT_MASK 0x3

const uint8_t MAX_BATTERIES = 10;

#endif