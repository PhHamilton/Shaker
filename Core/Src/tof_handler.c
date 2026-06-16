#include "tof_handler.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "VL53L0X.h"

extern UART_HandleTypeDef huart1;

#define TOF_RX_BUF_SIZE 50

static uint8_t rx_buf[TOF_RX_BUF_SIZE];
static uint32_t calibration_start_tick = 0;
static uint8_t increment_counter = 1;
static uint32_t calibration_sum = 0;
static uint32_t calibration_measurement_counter = 0;

static tof_handler_status_t tof_status = TOF_IDLE;
static tof_calibration_state_t calibration_state = TOF_CALIBRATION_START;

static void enable_module(void)
{
    HAL_GPIO_WritePin(VL0x_XSHUT_GPIO_Port, VL0x_XSHUT_Pin, GPIO_PIN_SET);
}

tof_handler_status_t tof_handler_init(tof_handler_t *tof_handler)
{
    if(tof_handler->calibration_param.x0 != 0 &&
       tof_handler->calibration_param.y0 != 0 &&
       tof_handler->calibration_param.x1 != 0 &&
       tof_handler->calibration_param.y1 != 0)
    {
        tof_handler->calibration_completed = true;
    }
    else
    {
        tof_handler->calibration_param.x0 = 0;
        tof_handler->calibration_param.y0 = 0;
        tof_handler->calibration_param.x1 = 0;
        tof_handler->calibration_param.y1 = 0;
        tof_handler->calibration_completed = false;
    }

    enable_module();
    HAL_Delay(500);

    tof_handler->x_mm = 0;

    if(!initVL53L0X(1))
    {
        return TOF_ERROR;
    }

    return TOF_OK;
}

tof_calibration_state_t tof_handler_calibrate(tof_handler_t *tof_handler)
{
    switch(calibration_state)
    {
        case TOF_CALIBRATION_START:
        {
            tof_status = TOF_CALIBRATION;
            char tx_buf[] = "Y0 = ";
            HAL_UART_Transmit(&huart1, (uint8_t*)tx_buf, sizeof(tx_buf), 100);
            calibration_state = TOF_CALIBRATION_WAITING_Y0_CONFIRMATION;
        }
        break;
        case TOF_CALIBRATION_WAITING_Y0_CONFIRMATION:
        {
            HAL_UART_Receive(&huart1, rx_buf, sizeof(rx_buf), 10000);

            // Validate that the number is between 0-255;
            uint8_t y0 = rx_buf[0];
            if(y0 > 0 && y0 < 10)
            {
                tof_handler->calibration_param.y0 = y0;
                calibration_state = TOF_CALIBRATION_REQUEST_Y1;
            }
            else
            {
                return TOF_CALIBRATION_ERROR;
            }
        }
        break;
        case TOF_CALIBRATION_REQUEST_Y1:
        {
            char tx_buf[] = "Y1 = ";
            HAL_UART_Transmit(&huart1, (uint8_t*)tx_buf, sizeof(tx_buf), 100);
            calibration_state = TOF_CALIBRATION_WAITING_Y1_CONFIRMATION;
        }
        break;
        case TOF_CALIBRATION_WAITING_Y1_CONFIRMATION:
        {
            HAL_UART_Receive(&huart1, rx_buf, sizeof(rx_buf), 10000);

            // Validate that the number is between 0-255;
            uint8_t y1 = rx_buf[0];
            if(y1 > 0 && y1 < 10)
            {
                tof_handler->calibration_param.y1 = y1;
                calibration_state = TOF_CALIBRATION_PREPARE_X0_CALIBRATION;
            }
            else
            {
                return TOF_CALIBRATION_ERROR;
            }
        }
        break;
        case TOF_CALIBRATION_PREPARE_X0_CALIBRATION:
        {
            char tx_buf[TOF_RX_BUF_SIZE];
            snprintf(tx_buf, sizeof(tx_buf), "Place reflector %i mm", (uint8_t)tof_handler->calibration_param.y0);
            HAL_UART_Transmit(&huart1, (uint8_t*)tx_buf, sizeof(tx_buf), 100);

            memset(tx_buf, 0, sizeof(tx_buf));
            snprintf(tx_buf, sizeof(tx_buf), "Starting measurement in %i", TOF_CALIBRATION_PREPARATION_TIME_MS/1000);
            HAL_UART_Transmit(&huart1, (uint8_t*)tx_buf, sizeof(tx_buf), 100);

            calibration_state = TOF_CALIBRATION_WAIT_X0_START;
            calibration_start_tick = HAL_GetTick();
        }
        break;
        case TOF_CALIBRATION_WAIT_X0_START:
        {
            uint32_t elapsed_time = HAL_GetTick() - calibration_start_tick;
            if(elapsed_time > 1000 * increment_counter)
            {
                uint8_t tx_buf = TOF_CALIBRATION_PREPARATION_TIME_MS - 1000 * increment_counter++;
                HAL_UART_Transmit(&huart1, &tx_buf, sizeof(tx_buf), 100);
            }
            if(elapsed_time > TOF_CALIBRATION_PREPARATION_TIME_MS)
            {
                calibration_state = TOF_CALIBRATION_STARTING_X0_CALIBRATION;
                calibration_start_tick = HAL_GetTick();
            }
        }
        break;
        case TOF_CALIBRATION_STARTING_X0_CALIBRATION:
        {
            uint32_t elapsed_time = HAL_GetTick() - calibration_start_tick;
            if(elapsed_time > CALIBRATION_MEASUREMENT_DELAY)
            {
                calibration_sum += readRangeSingleMillimeters(0);
                calibration_measurement_counter++;
            }

            if(calibration_measurement_counter == N_CALIBRATION_MEASUREMENTS)
            {
                tof_handler->calibration_param.x0 = (int16_t)calibration_sum / N_CALIBRATION_MEASUREMENTS;
                calibration_state = TOF_CALIBRATION_PREPARE_X1_CALIBRATION;
            }
        }
        break;
        case TOF_CALIBRATION_PREPARE_X1_CALIBRATION:
        {
            char tx_buf[TOF_RX_BUF_SIZE];
            snprintf(tx_buf, sizeof(tx_buf), "Place reflector %i mm", tof_handler->calibration_param.y1);
            HAL_UART_Transmit(&huart1, (uint8_t*)tx_buf, sizeof(tx_buf), 100);

            memset(&tx_buf[0], 0, sizeof(tx_buf));
            snprintf(tx_buf, sizeof(tx_buf), "Starting measurement in %i", TOF_CALIBRATION_PREPARATION_TIME_MS/1000);
            HAL_UART_Transmit(&huart1, (uint8_t*)tx_buf, sizeof(tx_buf), 100);
            increment_counter = 0;

            calibration_state = TOF_CALIBRATION_WAIT_X1_START;
            calibration_start_tick = HAL_GetTick();
        }
        break;
        case TOF_CALIBRATION_WAIT_X1_START:
        {
            uint32_t elapsed_time = HAL_GetTick() - calibration_start_tick;
            if(elapsed_time > 1000 * increment_counter)
            {
                uint8_t tx_buf = TOF_CALIBRATION_PREPARATION_TIME_MS - 1000 * increment_counter++;
                HAL_UART_Transmit(&huart1, &tx_buf, sizeof(tx_buf), 100);
            }
            if(elapsed_time > TOF_CALIBRATION_PREPARATION_TIME_MS)
            {
                calibration_state = TOF_CALIBRATION_STARTING_X1_CALIBRATION;
                calibration_measurement_counter = 1;
                calibration_sum = 0;
                calibration_start_tick = HAL_GetTick();
            }
        }
        break;
        case TOF_CALIBRATION_STARTING_X1_CALIBRATION:
        {
            uint32_t elapsed_time = HAL_GetTick() - calibration_start_tick;
            if(elapsed_time > CALIBRATION_MEASUREMENT_DELAY)
            {
                calibration_sum += readRangeSingleMillimeters(0);
                calibration_measurement_counter++;
            }

            if(calibration_measurement_counter == N_CALIBRATION_MEASUREMENTS)
            {
                tof_handler->calibration_param.x1 = (int16_t)calibration_sum / N_CALIBRATION_MEASUREMENTS;
                calibration_state = TOF_CALIBRATION_COMPLETED;
            }
        }
        break;
        case TOF_CALIBRATION_COMPLETED:
        {

        }
        break;
        case TOF_CALIBRATION_ERROR:
        {

        }
        break;
    }

    return calibration_state;
}

tof_handler_status_t tof_handler_start_measurement(tof_handler_measurement_frequency fs)
{
    startContinuous(fs);
    tof_status = TOF_MEASURING;
    return TOF_OK;
}

tof_handler_status_t tof_handler_stop_measurement(void)
{
    stopContinuous();
    tof_status = TOF_IDLE;
    return TOF_OK;
}
static uint16_t map_to_calibration(tof_handler_t *tof_handler, uint16_t measurement)
{
/* Linear Mapping:
    y = kx + m

    y0 = k * x0 + m --> m = y0 - k * x0
    y1 = k * x1 + m

    y1 = k * x1 + y0 - k*x0

    y1-y0 = k(x1-x0) --> k = (y1-y0)/(x1-x0)

    m = y0 - k * x0 --> m = y0 - (y1-y0)/(x1-x0) * x0

    y = (y1-y0)/(x1-x0)*x + y0 - (y1-y0)(x1-x0) * x0
    = (y1-y0)/(x1-x0) * (x - x0) + y0
*/
      return (measurement-90) * 50 / 54 + 50;
//    return (tof_handler->calibration_param.y1 - tof_handler->calibration_param.y0) / (tof_handler->calibration_param.x1 - tof_handler->calibration_param.x0) * (measurement - tof_handler->calibration_param.x0) + tof_handler->calibration_param.y0;
}
void tof_handler_get_latest_measurement(tof_handler_t *tof_handler)
{
    uint16_t x_meas = readRangeContinuousMillimeters(0);
    tof_handler->x_mm = map_to_calibration(tof_handler, x_meas);
}

tof_handler_status_t tof_handler_get_status(tof_handler_t *tof_handler)
{
    return tof_status;
}
