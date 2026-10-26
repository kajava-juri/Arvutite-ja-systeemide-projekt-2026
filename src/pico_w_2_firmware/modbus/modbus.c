/* 
 * Modbus Application layer implementation
 *
 * 
*/
#include "modbus.h"
#include "modbus_function_codes.h"
#include <string.h>

static ModbusStatus map_rtu_error_to_modbus_status(ModbusRtuError error)
{
    switch (error) {
        case MODBUS_RTU_ERROR_NONE: return MB_STATUS_OK;
        case MODBUS_RTU_ERROR_TIMEOUT: return MB_TIMEOUT;
        case MODBUS_RTU_ERROR_MAX_RETRIES_EXCEEDED: return MB_RETRIES_EXCEEDED;
        case MODBUS_RTU_TRANSPORT_ERROR: return MB_TRANSPORT_ERROR;
        default: return MB_ERROR;
    }
}

static char *modbus_status_to_string(ModbusStatus status)
{
    switch (status) {
        case MB_STATUS_OK: return "OK";
        case MB_BUSY: return "BUSY";
        case MB_TIMEOUT: return "TIMEOUT";
        case MB_ERROR: return "ERROR";
        case MB_TRANSPORT_ERROR: return "TRANSPORT_ERROR";
        case MB_RETRIES_EXCEEDED: return "RETRIES_EXCEEDED";
        case MB_SLAVE_EXCEPTION: return "SLAVE_EXCEPTION";
        case MB_INVALID_FUNCTION_CODE: return "INVALID_FUNCTION_CODE";
        default: return "UNKNOWN_STATUS";
    }
}

static ModbusStatus parse_read_registers(ModbusContext *ctx, const uint8_t *pdu, size_t len, ModbusResult *res)
{
    uint8_t byte_count = pdu[1];

    // char debug_msg[128];
    // snprintf(debug_msg, sizeof(debug_msg), "Parsing read registers: byte_count=%u, len=%zu, expected_count=%u\n", byte_count, len, 2 * ctx->expected_count);
    // uart_puts(UART_ID, debug_msg);
    
    if (byte_count != 2 * ctx->expected_count) return MB_ERROR;

    for (uint16_t i = 0; i < ctx->expected_count; i++) {
        ctx->regs[i] = ((uint16_t)pdu[2 + 2*i] << 8) | pdu[3 + 2*i];
        // ctx->regs[i] = ((uint16_t)(uint8_t)pdu[2*i] << 8) | (uint8_t)pdu[2*i + 1];
    }

    res->regs = ctx->regs;
    res->reg_count = ctx->expected_count;
    return MB_STATUS_OK;
}

static ModbusStatus parse_reply(ModbusContext *ctx, uint8_t *pdu, size_t len, ModbusResult *res)
{
    if (len < 2) return MB_ERROR;

    uint8_t function_code = pdu[0];
    uint8_t request_function_code = ctx->rtu.request.function_code;

    if (function_code == (request_function_code | 0x80)) {
        res->exception_code = pdu[1];
        return MB_SLAVE_EXCEPTION;
    }
    if (function_code != request_function_code) return MB_INVALID_FUNCTION_CODE;

    switch (function_code) {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
            return parse_read_registers(ctx, pdu, len, res);
        default:
            return MB_UNSUPPORTED_FUNCTION;
    }
}

ModbusStatus modbus_read_from_holding_registers(ModbusContext *ctx, uint16_t register_address, uint16_t register_count, uint8_t slave_address,
    ModbusCallback callback, void *user_data)
{
    // only able to send in IDLE state
    if (ctx->rtu.state != MODBUS_RTU_IDLE) {
        return MB_BUSY;
    }
    
    char data[4];
    data[0] = (register_address >> 8) & 0xFF;
    data[1] = register_address & 0xFF;
    data[2] = (register_count >> 8) & 0xFF;
    data[3] = register_count & 0xFF;
    ctx->expected_count = register_count;
    modbus_rtu_set_slave_address(&ctx->rtu, slave_address);
    modbus_rtu_build_request(&ctx->rtu, MODBUS_FC_READ_HOLDING_REGISTERS, data, 4);
    if(modbus_rtu_send_request(&ctx->rtu) < 0) {
        // this function forwards the return of the transport.write
        return MB_TRANSPORT_ERROR;
    }
    
    ctx->callback = callback;
    ctx->user_data = user_data;

    return MB_STATUS_OK;
}

ModbusStatus modbus_task(ModbusContext *ctx)
{
    ModbusRtuError error = modbus_rtu_state_machine(&ctx->rtu);
    ModbusStatus status = map_rtu_error_to_modbus_status(error);

    ModbusRtuError err;
    if(!modbus_rtu_take_result(&ctx->rtu, &err)) {
        return status;
    }
    ModbusResult res = { .status = map_rtu_error_to_modbus_status(err) };

    if (err == MODBUS_RTU_ERROR_NONE) {
        res.status = parse_reply(ctx, (uint8_t *)ctx->rtu.reply.data, ctx->rtu.reply.data_length, &res);
        if( res.status != MB_STATUS_OK) {
            uart_puts(UART_ID, "Error parsing reply PDU: ");
            uart_puts(UART_ID, modbus_status_to_string(res.status));
            uart_puts(UART_ID, "\n");
        }
        // const char *d = ctx->rtu.reply.data;     // register bytes, big-endian
        // res.reg_count = ctx->rtu.reply.data_length / 2;
        // for (uint16_t i = 0; i < res.reg_count; i++) {
        //     ctx->regs[i] = ((uint16_t)(uint8_t)d[2*i] << 8) | (uint8_t)d[2*i + 1];
        // }
        // res.regs = ctx->regs;
    } else if (err >= MB_STATUS_ERROR_START)
    {
        uart_puts(UART_ID, "Modbus error occurred: ");
        uart_puts(UART_ID, modbus_status_to_string(map_rtu_error_to_modbus_status(err)));
        uart_puts(UART_ID, "\n");
    }
    

    ModbusCallback cb = ctx->callback;
    void *user = ctx->user_data;
    ctx->callback = NULL;              // cleared first, so the callback may start a new request
    if (cb) cb(&res, user);
}

int modbus_is_busy(ModbusContext *context)
{
    return context->rtu.state != MODBUS_RTU_IDLE;
}
