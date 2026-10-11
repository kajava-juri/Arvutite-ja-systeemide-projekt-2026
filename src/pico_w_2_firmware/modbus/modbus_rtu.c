#include "modbus_rtu.h"
#include "modbus_function_codes.h"
#include "drivers/max485.h"
#include <string.h>

int one_request_sent = 0;

// for why the CRC table values are the way they are, refer to modbus specification
// https://www.modbus.org/file/secure/modbusoverserial.pdf

/* Table of CRC values for high–order byte */
static unsigned char auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
};

/* Table of CRC values for low–order byte */
static char auchCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};

uint16_t compute_crc_simple(uint8_t *message, size_t message_length)
{
    uint16_t crc_reg = 0xFFFF;
    uint16_t polynomial = 0xA001;

    // Exclusive OR the first 8–bit byte of the message with the low–order byte of the 16–bit CRC register, putting the result in the 
    // CRC register.
    // printf("message[0] = 0x%02X\n", message[0]);
    uint8_t lsb;
    for(int i = 0; i < message_length; i++) {
        crc_reg ^= message[i];
        for(int j = 0; j < 8; j++) {    
            lsb = crc_reg & 0x01;
            crc_reg >>= 1;
            if (lsb) {
                crc_reg ^= polynomial;
            }
        }
    }

    return crc_reg;
}

// function code taken from https://www.modbus.org/file/secure/modbusoverserial.pdf
uint16_t compute_crc_fast(uint8_t *message, size_t message_length)
{
    unsigned char uchCRCHi = 0xFF; /* high byte of CRC initialized */
    unsigned char uchCRCLo = 0xFF; /* low byte of CRC initialized */
    unsigned uIndex ; /* will index into CRC lookup table */
    while (message_length--) /* pass through message buffer */
    {
        uIndex = uchCRCLo ^ *message++ ; /* calculate the CRC */
        uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex] ;
        uchCRCHi = auchCRCLo[uIndex] ;
    }
    return (uchCRCHi << 8 | uchCRCLo);
}

void modbus_init(ModbusRtuContext *ctx)
{
    ctx->state = MODBUS_RTU_IDLE;
    ctx->pending_error = MODBUS_RTU_ERROR_NONE;
    ctx->retry_count = 0;
}

void modbus_action_idle(ModbusRtuContext *ctx)
{
    uart_puts(UART_ID, "Modbus RTU Idle state\n");
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if(current_time - ctx->last_request_time < POLLING_INTERVAL_MS) {
        return; // not enough time has passed since last request
    }

    modbus_read_from_holding_registers(ctx, 0x0000, 3);

    ctx->state = MODBUS_RTU_WAITING_FOR_REPLY;
    uart_puts(UART_ID, "Waiting for reply from slave\n");
}

void modbus_action_waiting_for_reply(ModbusRtuContext *ctx)
{
    uart_puts(UART_ID, "Waiting for reply state\n");

    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - ctx->last_request_time > MODBUS_RTU_REPLY_TIMEOUT_MS) {
        uart_puts(UART_ID, "Timeout waiting for reply\n");
        ctx->state = MODBUS_RTU_PROCESSING_ERROR;
        ctx->pending_error = MODBUS_RTU_ERROR_TIMEOUT;
        return;
    }

    int res;
    // read function code
    res = max485_receive_data((uint8_t *)&ctx->reply.data, MODBUS_RTU_FRAME_START_END_DELAY_MS * 1000, sizeof(ctx->reply.data));

    if (res == 0) {
        ctx->retry_count++;
        if (ctx->retry_count > MODBUS_RTU_MAX_RETRIES) {
            ctx->state = MODBUS_RTU_PROCESSING_ERROR;
            ctx->pending_error = MODBUS_RTU_ERROR_MAX_RETRIES_EXCEEDED;
            return;
        }
        return; // timeout with no data received, stay in waiting for reply state
    }
    if (res < 0) {
        uart_puts(UART_ID, "Timeout or error receiving function code\n");
        ctx->state = MODBUS_RTU_PROCESSING_ERROR;
        return;
    }
    ctx->reply.data_length = res;
    if (ctx->reply.data_length < 2) {
        uart_puts(UART_ID, "Received data length is too short\n");
        ctx->state = MODBUS_RTU_PROCESSING_ERROR;
        return;
    }
    ctx->reply.slave_address = ctx->reply.data[0];
    ctx->reply.function_code = ctx->reply.data[1];
    if (ctx->reply.function_code == ctx->request.function_code + 0x80) {
        uart_puts(UART_ID, "Error response from slave\n");
        ctx->state = MODBUS_RTU_PROCESSING_ERROR;
        return;
    }

    if (ctx->reply.function_code != ctx->request.function_code) {
        uart_puts(UART_ID, "Invalid function code in reply\n");
        ctx->reply.exception_code = ctx->reply.data[2];
        ctx->state = MODBUS_RTU_PROCESSING_ERROR;
        return;
    }

    if (ctx->reply.data_length < 3) {
        uart_puts(UART_ID, "Received data length is too short\n");
        ctx->state = MODBUS_RTU_PROCESSING_ERROR;
        return;
    }
    ctx->reply.data_length = ctx->reply.data[2];

    uint16_t received_crc = (uint8_t)ctx->reply.data[3 + ctx->reply.data_length] | ((uint8_t)ctx->reply.data[4 + ctx->reply.data_length] << 8);
    uint16_t computed_crc = compute_crc_fast((uint8_t *)&ctx->reply.data, 3 + ctx->reply.data_length);

    if (received_crc != computed_crc) {
        uart_puts(UART_ID, "Invalid CRC in reply\n");
        ctx->state = MODBUS_RTU_PROCESSING_ERROR;
        return;
    }

    // update data pointer to point to the actual data, skipping slave address and function code
    memmove(ctx->reply.data, ctx->reply.data + 3, ctx->reply.data_length);

    ctx->state = MODBUS_RTU_PROCESSING_REPLY;
}

void modbus_action_processing_reply(ModbusRtuContext *ctx)
{
    uart_puts(UART_ID, "Processing reply state\n");

    char formatted_data[512];
    snprintf(formatted_data, sizeof(formatted_data), "Reply from slave:\nSlave address: %d\nFunction code: %d\nData length: %d\nData: ", ctx->reply.slave_address, ctx->reply.function_code, ctx->reply.data_length);
    for (int i = 0; i < ctx->reply.data_length; i++) {
        char byte_str[4];
        snprintf(byte_str, sizeof(byte_str), "%02X ", (unsigned char)ctx->reply.data[i]);
        strncat(formatted_data, byte_str, sizeof(formatted_data) - strlen(formatted_data) - 1);
    }
    strncat(formatted_data, "\n", sizeof(formatted_data) - strlen(formatted_data) - 1);
    uart_puts(UART_ID, formatted_data);

    ctx->state = MODBUS_RTU_IDLE;
}

void modbus_action_processing_error(ModbusRtuContext *ctx)
{
    uint8_t error_code = ctx->request.function_code;
    uint8_t exception_code = ctx->reply.exception_code;
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

    return 0;
}

void modbus_frame_serialize(ModbusRtuFrame *frame)
{
    uint16_t crc_value = compute_crc_fast((uint8_t *)frame, 2 + frame->data_length);
    frame->data[frame->data_length]     = crc_value & 0xFF;   // low byte first
    frame->data[frame->data_length + 1] = (crc_value >> 8) & 0xFF;
}


void modbus_send_request(ModbusRtuContext *ctx)
{
    modbus_frame_serialize(&ctx->request);
    sleep_ms(MODBUS_RTU_FRAME_START_END_DELAY_MS);
    // later call ensure_rtu_frame_start_end_delay()

    max485_send_data((uint8_t *)&ctx->request, 2 + ctx->request.data_length + 2); // slave address + function code + data + CRC
    ctx->last_request_time = to_ms_since_boot(get_absolute_time());
}

int modbus_state_machine(ModbusRtuContext *ctx)
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

    return ctx->pending_error;
}

void modbus_read_from_holding_registers(ModbusRtuContext *ctx, uint16_t register_address, uint16_t register_count)
{
    char data[4];
    data[0] = (register_address >> 8) & 0xFF;
    data[1] = register_address & 0xFF;
    data[2] = (register_count >> 8) & 0xFF;
    data[3] = register_count & 0xFF;
    modbus_set_slave_address(ctx, (uint8_t)42);
    modbus_build_request(ctx, MODBUS_FC_READ_HOLDING_REGISTERS, data, 4);
    modbus_send_request(ctx);
}
