#ifndef MODBUS_INTERFACE_H
#define MODBUS_INTERFACE_H

#define INTERFACE_REG_PRESSURE 1

typedef struct {
    ModbusContext *mb;
    uint8_t        slave;
    bool           pending;
    uint32_t       last_request_ms;

    float          pressure;
    uint32_t       updated_ms;
    

    ModbusStatus   last_status;
} InterfaceSlave;

#endif // MODBUS_INTERFACE_H