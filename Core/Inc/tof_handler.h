#ifndef __TOF_HANDLER_H__
#define __TOF_HANDLER_H__

#include <stdint.h>
#include <stdbool.h>

#define TOF_CALIBRATION_PREPARATION_TIME_MS 10000
#define CALIBRATION_MEASUREMENT_DELAY 50
#define N_CALIBRATION_MEASUREMENTS 100

typedef enum
{
    TOF_CALIBRATION_START,
    TOF_CALIBRATION_WAITING_Y0_CONFIRMATION,
    TOF_CALIBRATION_REQUEST_Y1,
    TOF_CALIBRATION_WAITING_Y1_CONFIRMATION,
    TOF_CALIBRATION_PREPARE_X0_CALIBRATION,
    TOF_CALIBRATION_WAIT_X0_START,
    TOF_CALIBRATION_STARTING_X0_CALIBRATION,
    TOF_CALIBRATION_PREPARE_X1_CALIBRATION,
    TOF_CALIBRATION_WAIT_X1_START,
    TOF_CALIBRATION_STARTING_X1_CALIBRATION,
    TOF_CALIBRATION_COMPLETED,
    TOF_CALIBRATION_ERROR
}tof_calibration_state_t;

typedef enum
{
    TOF_OK,
    TOF_IDLE,
    TOF_CALIBRATION,
    TOF_MEASURING,
    TOF_ERROR
}tof_handler_status_t;

typedef enum
{
    TOF_MEASUREMENT_ASAP = 0,
    TOF_MEASUREMENT_1HZ  = 1000,
    TOF_MEASUREMENT_5HZ  = 5,
    TOF_MEASUREMENT_10HZ = 10,
    TOF_MEASUREMENT_20HZ = 20,
    TOF_MEASUREMENT_30HZ = 30,
}tof_handler_measurement_frequency;

typedef struct
{
    struct
    {
        uint16_t x0;
        uint16_t y0;
        uint16_t x1;
        uint16_t y1;
    }calibration_param;

    uint16_t x_mm;
    bool calibration_completed;
}tof_handler_t;

tof_handler_status_t tof_handler_init(tof_handler_t *tof_handler);
tof_calibration_state_t tof_handler_calibrate(tof_handler_t *tof_handler);
tof_handler_status_t tof_handler_start_measurement(tof_handler_measurement_frequency fs);
tof_handler_status_t tof_handler_stop_measurement(void);
void tof_handler_get_latest_measurement(tof_handler_t *tof_handler);
tof_handler_status_t tof_handler_get_status(tof_handler_t *tof_handler);

#endif //__TOF_HANDLER_H__
