#ifndef QBM68_H
#define QBM68_H

#include "modbus/modbus.h"

#define QBM68_REG_PRESSURE    4   // from datasheet
#define QBM68_POLL_MS         1000

typedef struct {
    ModbusContext *mb;
    uint8_t        slave;
    bool           pending;
    uint32_t       last_request_ms;

    int16_t        pressure_raw;
    float          pressure;
    uint32_t       updated_ms;
    ModbusStatus   last_status;
} Qbm68;

void qbm68_task(Qbm68 *dev);
float get_pressure_data(Qbm68 *dev);

#endif // QBM68_H