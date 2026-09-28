#ifndef MAX485_H
#define MAX485_H

#include "common.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h> 

typedef enum {
    RECEIVE = 0,
    TRANSMIT = 1
} Max485Mode;

// Transmitter state
typedef struct {
    Max485Mode mode;
    uart_inst_t *uart;
    bool initialized;
} Max485State;

extern Max485State max485_state;

void max485_init(uart_inst_t *uart);

void max485_set_transmit_mode(Max485Mode mode);

/**
 * Sends data over the MAX485 interface.
 * @param data Pointer to the data buffer to send.
 * @param length Length of the data to send.
 * @return 0 on success, -1 on failure.
 */
int max485_send_data(uint8_t* data, size_t length);

#endif // MAX485_H