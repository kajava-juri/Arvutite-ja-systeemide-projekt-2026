#include "max485.h"
#include "common.h"

Max485State max485_state = {0};

void max485_init(uart_inst_t *uart)
{
    max485_state.uart = uart;
    max485_state.initialized = true;
    max485_state.mode = RECEIVE;
}

void max485_set_transmit_mode(Max485Mode mode)
{
    gpio_put(RE_DE_PIN, (int)mode);
    max485_state.mode = mode;
}

int max485_send_data(uint8_t *data, size_t length)
{
    if (!max485_state.initialized) {
        uart_puts(UART_ID, "MAX485 not initialized\n");
        return -1; // not initialized
    }

    if (max485_state.mode != TRANSMIT) {
        uart_puts(UART_ID, "Not in transmit mode\n");
        return -1; // not in transmit mode
    }

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

    return 0;
}
