#include "modbus_interface.h"
#include "modbus/modbus.h"

static PublishData last_publish_data = {0};

static void on_write_register(ModbusResult *res, void *user)
{
    InterfaceSlave *dev = user;
    dev->pending     = false;
    dev->last_status = res->status;
    if (res->status != MB_STATUS_OK) return;

    uart_puts(UART_ID, "Register write successful\n");
}

void interface_slave_task(InterfaceSlave *dev)
{
    bool fresh = queue_try_remove(&dev->publish_q, &last_publish_data);
    if(fresh) {
        // FIX: check if the bus is busy before sending a new request
        // this causes the request to lose the publish data
        if(modbus_write_single_register(dev->mb, last_publish_data.reg_address, last_publish_data.reg_value, dev->slave,
                                      on_write_register, dev) == MB_STATUS_OK) {
            dev->pending = true;
            dev->last_request_ms = to_ms_since_boot(get_absolute_time());
        }
    }
}
