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
    uint re_de_pin;
} Max485State;

extern Max485State max485_state;

void max485_init(uart_inst_t *uart, uint re_de_pin);

void max485_set_transmit_mode(Max485Mode mode);

/**
 * Sends data over the MAX485 interface.
 * @param data Pointer to the data buffer to send.
 * @param length Length of the data to send.
 * @return 0 on success, -1 on failure.
 */
int max485_send_data(uint8_t* data, size_t length);

/**
 * Receives data over the MAX485 interface.
 * @param buffer Pointer to the buffer to store the received data.
 * @param length Length of the data to receive.
 * @param timeout Timeout of silence in microseconds. blocks until data is received or timeout 
 * is reached. If timeout is 0, it will block indefinitely until data is received.
 * @return 0 on timeout, positive means the number of bytes received, -1 on failure.
 */
int max485_receive_data(uint8_t* buffer, int timeout_us, size_t length);

#endif // MAX485_H