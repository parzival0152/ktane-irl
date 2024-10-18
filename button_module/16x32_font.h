// based on: https://github.com/idispatch/raster-fonts/blob/master/font-16x32.c

#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

extern const uint8_t FONT_CHAR_WIDTH;
extern const uint8_t FONT_CHAR_HEIGHT;
extern const uint16_t FONT_CHAR_NUM;

// char NON_FULL_ASCII_ORDER[] = {
//     ' ', '\'',
//     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
//     '?',
//     'A', /*...*/ 'Z'
// };

extern const uint32_t console_font_16x32[];

#endif /* FONT_H_ */