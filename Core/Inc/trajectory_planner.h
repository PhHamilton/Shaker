#ifndef __TRAJECTORY_PLANNER_H__
#define __TRAJECTORY_PLANNER_H__

#include <stdint.h>
#include <stdbool.h>
#include "tb6600.h"

typedef struct
{
    uint8_t j_max;
    uint8_t a_max;
    uint8_t v_max;
    int8_t d;
}test_suite_t;

typedef struct
{
    float j;
    float a;
    float v;
    float d;

    float j_cmd;
    float target_d;
    float steps_per_mm;

    bool is_running;

}motion_state_t;

typedef enum
{
    MOTION_OK,
    MOTION_RUNNING
}motion_status_t;

motion_status_t motion_init(motion_state_t *motion, float dt);
motion_status_t motion_update(motion_state_t *motion, test_suite_t *ts);

#endif
