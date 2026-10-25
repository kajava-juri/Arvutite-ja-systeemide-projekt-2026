#ifndef MAIN_H
#define MAIN_H

#include "common.h"

#define ERROR_BLINK_MS 250

typedef struct {
    float data;
    uint32_t updated_ms;
} SensorData;

void init_gpio_pins();
void core1_entry();

#endif // MAIN_H