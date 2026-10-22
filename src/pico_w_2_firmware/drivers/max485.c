#include "max485.h"
#include "common.h"

// Max485State max485_state = {0};

void max485_init(Max485State *dev, uart_inst_t *uart, uint re_de_pin)
{
    gpio_init(re_de_pin);
    gpio_set_dir(re_de_pin, GPIO_OUT);

    dev->re_de_pin = re_de_pin;
    dev->uart = uart;
    dev->initialized = true;
    dev->mode = RECEIVE;
}

void max485_set_transmit_mode(Max485State *dev, Max485Mode mode)
{
    gpio_put(dev->re_de_pin, (int)mode);
    dev->mode = mode;
}

int max485_send_data(Max485State *dev, uint8_t *data, size_t length)
{
    if (!dev->initialized) {
        uart_puts(UART_ID, "MAX485 not initialized\n");
        return -1; // not initialized
    }

    max485_set_transmit_mode(dev, TRANSMIT);

    if (!data || length == 0) {
        uart_puts(UART_ID, "Invalid data to send\n");
        return -1; // invalid data
    }

    if (uart_is_writable(dev->uart)) {
        uart_puts(UART_ID, "Sending data over MAX485\n");
        // uart_puts(dev->uart, "Hello, MAX485!\n");
        uart_write_blocking(dev->uart, data, length);
        // debug write to UART0
        uart_puts(UART_ID, "Data sent over MAX485\n");
    } else {
        uart_puts(UART_ID, "UART not writable\n");
        return -1; // UART not writable
    }

    uart_tx_wait_blocking(dev->uart);
    
    sleep_us(120);

    max485_set_transmit_mode(dev, RECEIVE);

    return 0;
}

int max485_receive_data(Max485State *dev, uint8_t *buffer, int timeout_us, size_t length)
{
    if (!dev->initialized) {
        uart_puts(UART_ID, "MAX485 not initialized\n");
        return -1; // not initialized
    }

    max485_set_transmit_mode(dev, RECEIVE);

    if (!buffer || length == 0) {
        uart_puts(UART_ID, "Invalid buffer to receive data\n");
        return -1; // invalid buffer
    }

    size_t bytes_received = 0;

    while (bytes_received < length) {
        if (uart_is_readable_within_us(dev->uart, timeout_us)) {
            buffer[bytes_received++] = uart_getc(dev->uart);
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

