#ifndef MODBUS_INTERFACE_H
#define MODBUS_INTERFACE_H

#include "pico/util/queue.h"
#include "modbus/modbus.h"

#define INTERFACE_REG_PRESSURE 1

typedef struct {
    uint16_t reg_address;
    uint16_t reg_value;
} PublishData;

typedef struct {
    ModbusContext   *mb;
    uint8_t         slave;
    bool            pending;
    uint32_t        last_request_ms;

    queue_t         publish_q; // Queue items must be of type PublishData`
    uint            queue_size;

    ModbusStatus   last_status;
} InterfaceSlave;

void interface_slave_task(InterfaceSlave *dev);

#endif // MODBUS_INTERFACE_H