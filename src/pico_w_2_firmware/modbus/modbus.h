#ifndef MODBUS_H
#define MODBUS_H

#include "modbus_rtu.h"

#define MB_STATUS_ERROR_START 200

typedef enum {
    MB_STATUS_OK = 0, MB_BUSY, 
    MB_TIMEOUT = MB_STATUS_ERROR_START, MB_ERROR, MB_TRANSPORT_ERROR, MB_RETRIES_EXCEEDED, MB_SLAVE_EXCEPTION,
    MB_INVALID_FUNCTION_CODE, MB_UNSUPPORTED_FUNCTION
} ModbusStatus;

typedef struct {
    ModbusStatus status;
    uint8_t exception_code;
    const uint16_t *regs;
    uint16_t reg_count;
} ModbusResult;

typedef void (*ModbusCallback)(ModbusResult *res, void *user_data);

// one abstraction layer above the ModbusRtuContext, to allow for future expansion to other Modbus types (hopefully right design choice)
// 'embeds' the rtu context into the struct
typedef struct {
    ModbusRtuContext rtu;
    ModbusCallback   callback;
    void            *user_data;
    uint16_t regs[125];
    uint8_t expected_count;
} ModbusContext;

ModbusStatus modbus_read_from_holding_registers(ModbusContext *context, uint16_t register_address, uint16_t register_count, uint8_t slave_address,
    ModbusCallback callback, void *user_data);

ModbusStatus modbus_task(ModbusContext *context);

int modbus_is_busy(ModbusContext *context);

#endif // MODBUS_H