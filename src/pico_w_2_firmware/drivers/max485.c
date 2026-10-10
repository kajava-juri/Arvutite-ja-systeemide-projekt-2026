#include "max485.h"
#include "common.h"

Max485State max485_state = {0};

void max485_init(uart_inst_t *uart, uint re_de_pin)
{
    gpio_init(re_de_pin);
    gpio_set_dir(re_de_pin, GPIO_OUT);

    max485_state.re_de_pin = re_de_pin;
    max485_state.uart = uart;
    max485_state.initialized = true;
    max485_state.mode = RECEIVE;
}

void max485_set_transmit_mode(Max485Mode mode)
{
    gpio_put(max485_state.re_de_pin, (int)mode);
    max485_state.mode = mode;
}

int max485_send_data(uint8_t *data, size_t length)
{
    if (!max485_state.initialized) {
        uart_puts(UART_ID, "MAX485 not initialized\n");
        return -1; // not initialized
    }

    // if (max485_state.mode != TRANSMIT) {
    //     uart_puts(UART_ID, "Not in transmit mode\n");
    //     return -1; // not in transmit mode
    // }

    max485_set_transmit_mode(TRANSMIT);

    if (!data || length == 0) {
        uart_puts(UART_ID, "Invalid data to send\n");
        return -1; // invalid data
    }

    if (uart_is_writable(max485_state.uart)) {
        uart_puts(UART_ID, "Sending data over MAX485\n");
        // uart_puts(max485_state.uart, "Hello, MAX485!\n");
        uart_write_blocking(max485_state.uart, data, length);
        // debug write to UART0
        uart_puts(UART_ID, "Data sent over MAX485\n");
    } else {
        uart_puts(UART_ID, "UART not writable\n");
        return -1; // UART not writable
    }

    uart_tx_wait_blocking(max485_state.uart);

    max485_set_transmit_mode(RECEIVE);

    return 0;
}

int max485_receive_data(uint8_t *buffer, int timeout_us, size_t length)
{
    if (!max485_state.initialized) {
        uart_puts(UART_ID, "MAX485 not initialized\n");
        return -1; // not initialized
    }

    max485_set_transmit_mode(RECEIVE);

    if (!buffer || length == 0) {
        uart_puts(UART_ID, "Invalid buffer to receive data\n");
        return -1; // invalid buffer
    }

    size_t bytes_received = 0;

    while (bytes_received < length) {
        if (uart_is_readable_within_us(max485_state.uart, timeout_us)) {
            buffer[bytes_received++] = uart_getc(max485_state.uart);
        } else {
            if (bytes_received > 0) {
                break;
            } else {
                uart_puts(UART_ID, "Timeout waiting for data\n");
                return 0; // timeout with no data received
            }
        }
    }

    return bytes_received; // return the number of bytes received
}

// void max485_receive_data(uint8_t *buffer, size_t length)
// {
//     max485_set_transmit_mode(RECEIVE);
//     if (uart_is_readable(max485_state.uart)) {
//         uart_puts(UART_ID, "Data received from slave\n");
//         // read data until the specified length is reached
//         size_t bytes_read = 0;
//         while (bytes_read < length) {
            
//         }
//     }
// }
