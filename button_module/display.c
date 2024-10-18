#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "display.h"

#define font console_font_16x32

void calc_render_area_buflen(render_area *area) {
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}


void SSD1306_send_cmd(uint8_t cmd) {
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(PERIPHERAL_I2C, SSD1306_I2C_ADDR, buf, 2, false);
}

void SSD1306_send_cmd_list(uint8_t *buf, int num) {
    for (int i=0;i<num;i++)
        SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen) {
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one gooooooo!

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    uint8_t *temp_buf = malloc(buflen + 1);

    temp_buf[0] = 0x40;
    memcpy(temp_buf+1, buf, buflen);

    i2c_write_blocking(PERIPHERAL_I2C, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}

void SSD1306_init() {
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t cmds[] = {
        SSD1306_SET_DISP,               // set display off
        /* memory mapping */
        SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
        0x00,                           // horizontal addressing mode
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // set display start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
        SSD1306_SET_MUX_RATIO,          // set multiplex ratio
        SSD1306_HEIGHT - 1,             // Display height - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
        SSD1306_SET_DISP_OFFSET,        // set display offset
        0x00,                           // no offset
        SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number.
                                        // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
#if ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 32))
        0x02,
#elif ((SSD1306_WIDTH == 128) && (SSD1306_HEIGHT == 64))
        0x12,
#else
        0x02,
#endif
        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
        0x80,                           // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,          // set pre-charge period
        0xF1,                           // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
        0x30,                           // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST,           // set contrast control
        0xFF,
        SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
        SSD1306_SET_CHARGE_PUMP,        // set charge pump
        0x14,                           // Vcc internally generated on our board
        SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
        SSD1306_SET_DISP | 0x01, // turn display on
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

static inline int GetFontIndex(uint8_t ch) {
    if (ch == ' ') {
        return 0;
    }
    else if (ch == '\'') {
        return 1;
    }
    else if (ch >= '0' && ch <='9') {
        return  ch - '0' + 2;
    }
    else if (ch == '?') {
        return 12;
    }
    else if (ch >= 'A' && ch <='Z') {
        return  ch - 'A' + 13;
    }
    else return 0; // Not got that char so space.
}

static void WriteChar(uint8_t *buf, uint16_t x, uint16_t y, uint8_t ch) {
    if (x > SSD1306_WIDTH - FONT_CHAR_WIDTH || y > SSD1306_HEIGHT - FONT_CHAR_HEIGHT)
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y / 8;

    ch = toupper(ch);
    int idx = GetFontIndex(ch);

    for (size_t row_idx=0; row_idx < (FONT_CHAR_HEIGHT + 7) / 8; row_idx++) {
        int fb_idx = (y + row_idx) * 128 + x;
        for (size_t cell_idx=0; cell_idx < FONT_CHAR_WIDTH; cell_idx++) {
            uint8_t bits_col = 0;
            switch (row_idx)
            {
            case 0:
                bits_col = 0x000000FF & font[idx * FONT_CHAR_WIDTH + cell_idx];
                break;
            case 1:
                bits_col = (0x0000FF00 & font[idx * FONT_CHAR_WIDTH + cell_idx]) >> 8;
                break;
            case 2:
                bits_col = (0x00FF0000 & font[idx * FONT_CHAR_WIDTH + cell_idx]) >> 16;
                break;
            case 3:
                bits_col = (0xFF000000 & font[idx * FONT_CHAR_WIDTH + cell_idx]) >> 24;
                break;
            }
            buf[fb_idx++] = bits_col;
        }
    }
}

void WriteString(uint8_t *buf, uint16_t x, uint16_t y, const char * str) {
    // Cull out any string off the screen
    if (x > SSD1306_WIDTH - FONT_CHAR_WIDTH || y > SSD1306_HEIGHT - FONT_CHAR_HEIGHT)
        return;

    while (*str) {
        WriteChar(buf, x, y, *str++);
        x += FONT_CHAR_WIDTH;
    }
}

void SSD1306_scroll(bool on) {
    // configure horizontal scrolling
    uint8_t cmds[] = {
        SSD1306_SET_HORIZ_SCROLL | 0x00,
        0x00, // dummy byte
        0x00, // start page 0
        0x00, // time interval
        0x03, // end page 3 SSD1306_NUM_PAGES ??
        0x00, // dummy byte
        0xFF, // dummy byte
        SSD1306_SET_SCROLL | (on ? 0x01 : 0) // Start/stop scrolling
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

void render(uint8_t *buf, render_area *area) {
    // update a portion of the display with a render area
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
    SSD1306_send_buf(buf, area->buflen);
}
