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
        default: return MB_ERROR;
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
        const char *d = ctx->rtu.reply.data;     // register bytes, big-endian
        res.reg_count = ctx->rtu.reply.data_length / 2;
        for (uint16_t i = 0; i < res.reg_count; i++) {
            ctx->regs[i] = ((uint16_t)(uint8_t)d[2*i] << 8) | (uint8_t)d[2*i + 1];
        }
        res.regs = ctx->regs;
    } else if (err == MODBUS_RTU_ERROR_SLAVE_EXCEPTION) {
        res.exception_code = ctx->rtu.reply.exception_code;
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
