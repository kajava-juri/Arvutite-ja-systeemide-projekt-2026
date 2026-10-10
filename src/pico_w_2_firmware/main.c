#include "main.h"

#include "common.h"
#include "drivers/max485.h"
#include <stdio.h>
#include "modbus/modbus_rtu.h"

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        uart_puts(UART_ID, "Wi-Fi init failed\n");
        return -1;
    }

    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);


    init_gpio_pins();
    max485_init(UART_ID_MAX485, RE_DE_PIN);
    max485_set_transmit_mode(TRANSMIT);
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart
    ModbusRtuContext ctx = {0};
    modbus_init(&ctx);

    while (true) {
        modbus_state_machine(&ctx);
        sleep_ms(MODBUS_RTU_FRAME_START_END_DELAY_MS * 2);
    }
}

void init_gpio_pins()
{
    // set up UART
    uart_init(UART_ID, BAUD_RATE);
    // set the TX and RX pins by using the function select on the GPIO
    // set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // UART for MAX485
    uart_init(UART_ID_MAX485, BAUD_RATE_MAX485);
    uart_set_format(UART_ID_MAX485, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(UART_ID_MAX485, false, false);
    gpio_set_function(UART_TX_PIN_MAX485, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN_MAX485, GPIO_FUNC_UART);

    uart_puts(UART_ID, "pins initialized\n");
}
