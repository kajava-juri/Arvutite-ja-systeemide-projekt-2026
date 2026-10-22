#include "main.h"

#include "common.h"
#include "drivers/max485.h"
#include <stdio.h>
#include "modbus/modbus_rtu.h"
#include "modbus/modbus.h"
#include <pico/multicore.h>
#include "pico/util/queue.h"
#include "devices/qbm68.h"

static int rtu_read(void *dev, uint8_t *buffer, size_t max_length)
{
    return max485_receive_data((Max485State *)dev, buffer, MODBUS_RTU_FRAME_START_END_DELAY_MS * 1000, max_length);
}

static int rtu_write(void *dev, uint8_t *data, size_t length)
{
    return max485_send_data((Max485State *)dev, data, length);
}

int main()
{
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        uart_puts(UART_ID, "Wi-Fi init failed\n");
        return -1;
    }

    // multicore_launch_core1(core1_entry);

    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    Max485State max485_state = {0};

    init_gpio_pins();


    max485_init(&max485_state, UART_ID_MAX485, RE_DE_PIN);
    max485_set_transmit_mode(&max485_state, RECEIVE);

    ModbusRtuContext ctx = {0};
    ModbusRtuTransport transport = {
        .write = rtu_write,
        .read = rtu_read,
        .dev = &max485_state
    };
    ctx.transport = transport;
    modbus_rtu_init(&ctx);

    ModbusContext mb = {
        .rtu = ctx,
        .regs = {0},
        .callback = NULL,
        .user_data = NULL
    };

    int modbus_res = 0;
    bool error_active = false;
    bool led_on = true;
    absolute_time_t next_toggle = make_timeout_time_ms(ERROR_BLINK_MS);

    Qbm68 qbm68_device = {
        .mb = &mb,
        .slave = 41,
        .pending = false,
        .last_request_ms = 0,
        .pressure_raw = 0,
        .pressure = 0.0f,
        .updated_ms = 0,
        .last_status = MB_STATUS_OK
    };

    while (true) {
        ModbusStatus modbus_res = modbus_task(&mb);
        qbm68_task(&qbm68_device);

        if (modbus_res == MB_RETRIES_EXCEEDED || modbus_res == MB_TIMEOUT || modbus_res == MB_TRANSPORT_ERROR) {
            error_active = true;
        } else if (modbus_res >= 0) {   // your "valid reply received" code
            error_active = false;
            led_on = true;                          // back to solid on
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
        }

        if (error_active && time_reached(next_toggle)) {
            led_on = !led_on;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
            next_toggle = make_timeout_time_ms(ERROR_BLINK_MS);
        }

        sleep_ms(MODBUS_RTU_FRAME_START_END_DELAY_MS);
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
