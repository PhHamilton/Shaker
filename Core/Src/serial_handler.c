#include "serial_handler.h"
#include "usart.h"

extern uart_msg_t uart_data;
extern bool uart_dma_transmission_completed;

static uint8_t msg_buf[MSG_BUF_SIZE];

static uint8_t uart_get_message(uint8_t *data, uint8_t max_size);
static uint16_t crc16_ccitt(const uint8_t *data, uint32_t len);

void serial_initialize(serial_packet_t *pkt)
{
    pkt->user_id = 0;
    pkt->cmd = SERIAL_UNKNOWN;
    pkt->payload_size = 0;
}

bool serial_message_available(void)
{
    return uart_data.msg > 0;
}

serial_status_t serial_parse(serial_packet_t *pkt)
{
    uint8_t n_bytes = uart_get_message(msg_buf, MSG_BUF_SIZE);

    if(n_bytes < MINIMUM_MSG_LENGTH || n_bytes > MSG_BUF_SIZE)
    {
        return SERIAL_INVALID_MSG;
    }

    uint8_t crc_low  = msg_buf[n_bytes - 3];
    uint8_t crc_high = msg_buf[n_bytes - 2];

    uint16_t computed_crc = crc16_ccitt(msg_buf + 2, n_bytes - 5); /* exclude SYNC1, SYNC2 */

    if(computed_crc != ((crc_high << 8) | crc_low))
    {
        return SERIAL_INVALID_MSG;
    }

    if(msg_buf[0] != SYNC1 || msg_buf[1] != SYNC2)
    {
        return SERIAL_INVALID_MSG;
    }

    pkt->user_id = (msg_buf[2] << 8) | msg_buf[3];

    if(pkt->user_id != ADMIN_USR_ID)
    {
        return SERIAL_INVALID_USER;
    }

    pkt->cmd = (serial_commands_t)msg_buf[4];

    pkt->payload_size = n_bytes - MINIMUM_MSG_LENGTH;
    pkt->payload = &msg_buf[5];

    return SERIAL_OK;
}

serial_status_t serial_send(const uint8_t *data, const uint8_t size)
{
    /* Frame: SYNC1 SYNC2 ID_HI ID_LO [payload x size] CRC_LO CRC_HI TERM = size+7 bytes */
    const uint8_t tx_data_size = size + 7;
    uint8_t tx_data[tx_data_size];

    tx_data[0] = SYNC1;
    tx_data[1] = SYNC2;
    tx_data[2] = (SHAKER_ID >> 8) & 0xFF;
    tx_data[3] = SHAKER_ID & 0xFF;

    for(uint8_t i = 0; i < size; i++)
    {
        tx_data[4+i] = data[i];
    }

    /* CRC covers ID_HI..end-of-payload, same as parser (excludes SYNC) */
    uint16_t crc = crc16_ccitt(&tx_data[2], size + 2);
    tx_data[4+size]   = crc & 0xFF;
    tx_data[4+size+1] = (crc >> 8) & 0xFF;
    tx_data[4+size+2] = SERIAL_MSG_TERMINATION;

    HAL_StatusTypeDef rc = HAL_UART_Transmit(&huart1, tx_data, sizeof(tx_data), 100);

    if(rc == HAL_BUSY)
    {
        return SERIAL_BUSY;
    }

    if(rc != HAL_OK)
    {
        return SERIAL_ERROR;
    }

    return SERIAL_OK;
}

static uint8_t uart_get_message(uint8_t *data, uint8_t max_size)
{
    if(!data || uart_data.msg == 0)
    {
        return 0;
    }

    uint8_t n_bytes = 0;

    while(uart_data.buffer_tail != uart_data.buffer_head)
    {
        uint8_t byte = uart_data.rx_buf[uart_data.buffer_tail];
        if(n_bytes < max_size)
        {
            data[n_bytes++] = byte;
        }

        uart_data.buffer_tail = (uart_data.buffer_tail + 1) % UART_BUFFER_SIZE;

        if(byte == SERIAL_MSG_TERMINATION)
        {
            uart_data.msg--;
            break;
        }
    }
    return n_bytes;
}

static uint16_t crc16_ccitt(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0xFFFF;

    for(uint32_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for(uint8_t b = 0; b < 8; ++b)
        {
            if(crc & 0x8000)
                crc = ((crc << 1)^0x1021) & 0xFFFF;
            else
                crc = (crc << 1) & 0xFFFF;
        }
    }

    return crc;
}
