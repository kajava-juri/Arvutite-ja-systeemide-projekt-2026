#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "common.h"

typedef enum
{
    MODBUS_RTU_PROCESSING_ERROR = 1,
    MODBUS_RTU_IDLE = 2,
    MODBUS_RTU_WAITING_FOR_REPLY = 3,
    MODBUS_RTU_PROCESSING_REPLY = 4,
    MODBUS_RTU_WAITING_TURNAROUND_DELAY = 5,
    MODBUS_RTU_MAX_STATES
} ModbusRtuState;

typedef enum {
    MODBUS_RTU_ERROR_NONE = 0,
    MODBUS_RTU_ERROR_TIMEOUT = -1,
    MODBUS_RTU_ERROR_INVALID_FUNCTION_CODE = -2,
    MODBUS_RTU_ERROR_INVALID_DATA_LENGTH = -3,
    MODBUS_RTU_ERROR_INVALID_CRC = -4,
    MODBUS_RTU_ERROR_SLAVE_EXCEPTION = -5,
    MODBUS_RTU_ERROR_MAX_RETRIES_EXCEEDED = -6,
} ModbusRtuError;

// start and end is defined as 3.5 character times
// for baudrate 9600, considering 8 data, 2 stop and 1 start bit = 11 bits
// (3.5 * 11) / 9600 = 4.01 ms
#define MODBUS_RTU_FRAME_START_END_DELAY_MS 5

#define MODBUS_RTU_REPLY_TIMEOUT_MS 1000
#define MODBUS_RTU_MAX_RETRIES 5
#define POLLING_INTERVAL_MS 1000

// each byte is sent least significant bit first
// contains 2 4 bit hex characters

// modbus rtu frame
// IMPORTANT! do not reorder the slave_address, function_code and data fields
// this order is used to calculate the CRC
typedef struct
{
    uint8_t slave_address;
    uint8_t function_code;
    char data[252 + 2]; // 252 bytes of data + 2 bytes for CRC
    uint8_t data_length;
    uint8_t exception_code;
} ModbusRtuFrame;

// modbus context structure
typedef struct
{
    ModbusRtuState state;
    ModbusRtuFrame request;
    ModbusRtuFrame reply;
    uint32_t last_request_time;
    ModbusRtuError pending_error;
    uint8_t retry_count;
} ModbusRtuContext;

void modbus_init(ModbusRtuContext *ctx);

/**
 * State "Idle" = no pending request. This is the initial state after power-up. A request can only be sent in "Idle" state. After sending 
 * a request, the Master leaves the "Idle" state, and cannot send a second request at the same time
 * @param state The current state of the Modbus RTU context
 */
void modbus_action_idle(ModbusRtuContext *ctx);

/**
 * When a unicast request is sent to a slave, the master goes into "Waiting for reply" state, and a “Response Time-out” is started. It 
 * prevents the Master from staying indefinitely in "Waiting for reply" state. Value of the Response time-out is application dependant.
 * 
 * When a reply is received, the Master checks the reply before starting the data processing. The checking may result in an error, 
 * for example a reply from an unexpected slave, or an error in the received frame. In case of a reply received from an
 * unexpected slave, the Response time-out is kept running. In case of an error detected on the frame, a retry may be performed.
 */
void modbus_action_waiting_for_reply(ModbusRtuContext *ctx);
void modbus_action_processing_reply(ModbusRtuContext *ctx);

/**
 * When a broadcast request is sent on the serial bus, no response is returned from the slaves. Nevertheless a delay is respected 
 * by the Master in order to allow any slave to process the current request before sending a new one. This delay is called 
 * "Turnaround delay". Therefore the master goes into "Waiting Turnaround delay" state before going back in "idle" state and before 
 * being able to send another request.
 */
void modbus_action_turnaround_delay(ModbusRtuContext *ctx);

/**
 * If no reply is received, the Response time-out expires, and an error is generated. Then the Master goes into "Idle" state, enabling 
 * a retry of the request. The maximum number of retries depends on the master set-up.
 */
void modbus_action_processing_error(ModbusRtuContext *ctx);

int modbus_set_slave_address(ModbusRtuContext *context, uint8_t slave_address);
int modbus_build_request(ModbusRtuContext *context, uint8_t function_code, const char *data, int data_length);

void modbus_send_request(ModbusRtuContext *context);
void modbus_read_request(ModbusRtuContext *context);

int modbus_state_machine(ModbusRtuContext *context);

void modbus_read_from_holding_registers(ModbusRtuContext *context, uint16_t register_address, uint16_t register_count);

void modbus_frame_serialize(ModbusRtuFrame *frame);

uint16_t compute_crc_fast(uint8_t *message, size_t message_length);
uint16_t compute_crc_simple(uint8_t *message, size_t message_length);

#endif // MODBUS_RTU_H