#ifndef __MOTOR_HANDLER_H__
#define __MOTOR_HANDLER_H__

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    TB6600_OK,
    TB6600_BUSY,
    TB6600_ERROR
}TB6600_status_t;

typedef enum
{
    TB6600_FORWARD = GPIO_PIN_SET,
    TB6600_REVERSE = GPIO_PIN_RESET
}TB6600_direction_t;

TB6600_status_t TB6600_init(void);
TB6600_status_t TB6600_enable(void);
TB6600_status_t TB6600_disable(void);
TB6600_status_t TB6600_set_direction(TB6600_direction_t dir);
TB6600_status_t TB6600_set_speed(uint32_t steps_per_sec);
TB6600_status_t TB6600_set_frequency(float frequency);
TB6600_status_t TB6600_start(TB6600_direction_t dir, uint32_t steps_per_sec);
TB6600_status_t TB6600_stop(void);

#endif //__MOTOR_HANDLER_H__
