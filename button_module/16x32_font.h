// based on: https://github.com/idispatch/raster-fonts/blob/master/font-16x32.c

#ifndef FONT_16X32_H_
#define FONT_16X32_H_

#include <stdint.h>

static const uint8_t FONT_CHAR_16x32_WIDTH = 16;
static const uint8_t FONT_CHAR_16x32_HEIGHT = 32;

extern const uint16_t FONT_CHAR_16X32_NUM;
extern const uint32_t CONSOLE_FONT_16X32[];

#endif /* FONT_16X32_H_ */