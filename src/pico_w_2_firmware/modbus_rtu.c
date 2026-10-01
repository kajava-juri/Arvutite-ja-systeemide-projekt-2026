#include "modbus_rtu.h"
#include "max485.h"
#include <string.h>

void modbus_init(ModbusRtuContext *ctx)
{
    ctx->state = MODBUS_RTU_IDLE;
}

void modbus_action_idle(ModbusRtuContext *ctx)
{
    uart_puts(UART_ID, "Modbus RTU Idle state\n");
    char *data = "Hello, Modbus!";
    modbus_set_slave_address(ctx, (uint8_t)42);
    modbus_build_request(ctx, 0x00, data, strlen(data));
    modbus_send_request(ctx);

    ctx->state = MODBUS_RTU_WAITING_FOR_REPLY;
}

void modbus_action_waiting_for_reply(ModbusRtuContext *ctx)
{
    uart_puts(UART_ID, "Waiting for reply from slave\n");
    max485_set_transmit_mode(RECEIVE);
    // check uart for incoming data
    if (uart_is_readable(UART_ID_MAX485)) {
        // read the reply frame from MAX485
        modbus_read_request(ctx);
        ctx->state = MODBUS_RTU_PROCESSING_REPLY;
    }
}

void modbus_action_processing_reply(ModbusRtuContext *ctx)
{

    uart_puts(UART_ID, "Processing reply from slave\n");
    uart_puts(UART_ID, "Slave address: ");
    uart_putc(UART_ID, ctx->reply.slave_address);
    uart_puts(UART_ID, "\nFunction code: ");
    uart_putc(UART_ID, ctx->reply.function_code);
    uart_puts(UART_ID, "\nData: ");
    uart_puts(UART_ID, ctx->reply.data);
    uart_puts(UART_ID, "\nData length: ");
    char data_length_str[4];
    snprintf(data_length_str, sizeof(data_length_str), "%d", ctx->reply.data_length);
    uart_puts(UART_ID, data_length_str);
    uart_puts(UART_ID, "\n");

    ctx->state = MODBUS_RTU_IDLE;
}

void modbus_action_processing_error(ModbusRtuContext *ctx)
{
    uart_puts(UART_ID, "Error processing reply from slave\n");
}

void modbus_action_turnaround_delay(ModbusRtuContext *ctx)
{
    uart_puts(UART_ID, "Waiting for turnaround delay\n");
}

int modbus_set_slave_address(ModbusRtuContext *ctx, uint8_t slave_address)
{
    ctx->request.slave_address = slave_address;
    return 0;
}

int modbus_build_request(ModbusRtuContext *ctx, uint8_t function_code, const char *data, int data_length)
{
    ctx->request.function_code = function_code;
    if (data_length > 252) {
        uart_puts(UART_ID, "Data length exceeds maximum allowed size\n");
        return -1; // data length exceeds maximum allowed size
    }
    ctx->request.data_length = data_length;
    memcpy(ctx->request.data, data, data_length);
    uint8_t crc[2] = {0x46, 0x2D};
    memcpy(ctx->request.crc, crc, 2);
    return 0;
}

void modbus_send_request(ModbusRtuContext *ctx)
{
    max485_set_transmit_mode(TRANSMIT);
    // Send the request frame over MAX485
    // send slave address
    //uart_write_blocking(UART_ID_MAX485, &ctx->request.slave_address, 1);;
    max485_send_data(&ctx->request.slave_address, 1);
    // send function code
    max485_send_data(&ctx->request.function_code, 1);
    // send data
    //uart_write_blocking(UART_ID_MAX485, (uint8_t *)ctx->request.data, ctx->request.data_length);
    max485_send_data((uint8_t *)ctx->request.data, ctx->request.data_length);
    // send CRC
    max485_send_data(ctx->request.crc, 2);

    max485_set_transmit_mode(RECEIVE);
}

void modbus_read_request(ModbusRtuContext *ctx)
{
    uart_read_blocking(UART_ID_MAX485, &ctx->reply.slave_address, 1);
    ctx->last_received_byte_time = to_ms_since_boot(get_absolute_time());

    uart_read_blocking(UART_ID_MAX485, &ctx->reply.function_code, 1);
    ctx->last_received_byte_time = to_ms_since_boot(get_absolute_time());
    
    uart_read_blocking(UART_ID_MAX485, (uint8_t *)ctx->reply.data, ctx->reply.data_length);
    ctx->last_received_byte_time = to_ms_since_boot(get_absolute_time());

    ctx->state = MODBUS_RTU_PROCESSING_REPLY;
}

void modbus_state_machine(ModbusRtuContext *ctx)
{
    switch (ctx->state) {
        case MODBUS_RTU_IDLE:
            modbus_action_idle(ctx);
            break;
        case MODBUS_RTU_WAITING_FOR_REPLY:
            modbus_action_waiting_for_reply(ctx);
            break;
        case MODBUS_RTU_PROCESSING_REPLY:
            modbus_action_processing_reply(ctx);
            break;
        case MODBUS_RTU_PROCESSING_ERROR:
            modbus_action_processing_error(ctx);
            break;
        case MODBUS_RTU_WAITING_TURNAROUND_DELAY:
            modbus_action_turnaround_delay(ctx);
            break;
    }
}
