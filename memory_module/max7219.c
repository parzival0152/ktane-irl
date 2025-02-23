#include <pico/stdlib.h>
#include <stdio.h>
#include <hardware/pio.h>
#include "max7219.pio.h"

PIO pio;
pio_sm_config smConfig;
uint sm;

void MAX7219_init(uint clk, uint dout, uint cs_n) {

    pio = pio0;
    sm = 0;

    uint offset = pio_add_program(pio, &max7219_program);

    smConfig = max7219_program_get_default_config(offset);

    pio_gpio_init(pio, clk);
    pio_gpio_init(pio, dout);
    pio_gpio_init(pio, cs_n);

    // set the data pin as the shift output pin
    sm_config_set_out_pins(&smConfig, dout, 1);
    sm_config_set_out_shift(&smConfig, false, false, 0);

    // set the chip select pin as the set pin
    sm_config_set_set_pins (&smConfig, cs_n, 1);

    // set the clock pin as the sideset pin
    sm_config_set_sideset_pins(&smConfig, clk);

    // set the clock divider to be at ~10MHz
    sm_config_set_clkdiv(&smConfig, 1500);
    
    pio_sm_init(pio, sm, offset, &smConfig);
    pio_sm_set_enabled(pio, sm, true);
}

void max7219_send_command(uint16_t data) {
    pio_sm_put(pio, sm, data<<16);
}