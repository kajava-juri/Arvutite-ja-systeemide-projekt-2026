#include "qbm68.h"
#include "modbus/modbus.h"

static void on_pressure(ModbusResult *res, void *user)
{
    Qbm68 *dev = user;
    dev->pending     = false;
    dev->last_status = res->status;
    if (res->status != MB_STATUS_OK || res->reg_count < 1) return;

    dev->pressure_raw = (int16_t)res->regs[0];   // signed register
    dev->pressure     = dev->pressure_raw;
    dev->updated_ms   = to_ms_since_boot(get_absolute_time());

    uart_puts(UART_ID, "Pressure read successfully\n");
    char formatted_data[512];
    snprintf(formatted_data, sizeof(formatted_data), "On pressure callback: Pressure raw: %d, Pressure: %.2f, Updated ms: %u\n", dev->pressure_raw, dev->pressure, dev->updated_ms);
    uart_puts(UART_ID, formatted_data);
}

void qbm68_task(Qbm68 *dev)
{
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (dev->pending || now - dev->last_request_ms < QBM68_POLL_MS) return;
// ModbusContext *ctx, uint16_t register_address, uint16_t register_count, uint8_t slave_address, ModbusCallback callback, void *user_data
    if (modbus_read_from_holding_registers(dev->mb, QBM68_REG_PRESSURE, 1, dev->slave,
                                      on_pressure, dev) == MB_STATUS_OK) {
        dev->pending = true;
        dev->last_request_ms = now;
    }
}