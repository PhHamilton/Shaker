#ifndef __SERIAL_HANDLER_H__
#define __SERIAL_HANDLER_H__

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define ADMIN_USR_ID 0xB055
#define TEST 0x00
#define SHAKER_ID 0x555B
#define MSG_BUF_SIZE 32
#define SYNC1 0xAA
#define SYNC2 0x55
#define SERIAL_MSG_TERMINATION 0x0A
#define MINIMUM_MSG_LENGTH 8 //SYNC1 + SYNC2 + ID01 + ID02 + CMD + CRC1 + CRC2 + SERIAL_MSG_TERMINATION

typedef enum
{
    SERIAL_IDENTIFY,
    SERIAL_CONFIGURE_PARAM,
    SERIAL_CONFIGURE_TEST_SUITE,
    SERIAL_START_TEST,
    SERIAL_STOP_TEST,
    SERIAL_UNKNOWN
}serial_commands_t;

typedef enum
{
    SERIAL_OK,
    SERIAL_BUSY,
    SERIAL_IDLE,
    SERIAL_INVALID_MSG,
    SERIAL_INVALID_USER,
    SERIAL_INVALID_COMMAND,
    SERIAL_ERROR
}serial_status_t;

typedef struct
{
    uint16_t user_id;
    serial_commands_t cmd;
    uint8_t *payload;
    uint8_t payload_size;
}serial_packet_t;

void serial_initialize(serial_packet_t *pkt);
bool serial_message_available(void);
serial_status_t serial_parse(serial_packet_t *pkt);

serial_status_t serial_send(const uint8_t *data, const uint8_t size);

void serial_rx_callback(uint8_t *data, uint16_t size);

#endif //__SERIAL_HANDLER_H__
