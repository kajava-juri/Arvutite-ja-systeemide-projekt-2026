#include <stdio.h>
#include "pico/stdlib.h"
#include "../common.h"
#include "rotary_encoder.h"

static int previous_value = 1;
static int value = 0;


static void rotary_changed(void) {
    int clk = gpio_get(CLK_PIN);   // read once so all checks see the same state

    if (previous_value != clk) {
        if (clk == 0) {
            if (gpio_get(DT_PIN) == 0) {
                value -= 1;
            } else {
                value += 1;
            }
        }
        previous_value = clk;
    }

    if (gpio_get(SW_PIN) == 0) {
        // printf("Button pressed\n");
        sleep_ms(1000);
    }
}

static void init_input(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

void rotary_main(void) {
    stdio_init_all();

    init_input(CLK_PIN);
    init_input(DT_PIN);
    init_input(SW_PIN);


    absolute_time_t next_print = make_timeout_time_ms(100);

    while (true) {
        rotary_changed();

        if (time_reached(next_print)) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "Rotary encoder value: %d\n", value);
            uart_puts(UART_ID, buffer);
            next_print = make_timeout_time_ms(100);
        }

        sleep_us(500);   // ~2 kHz sampling is plenty for a hand-turned knob
    }
}