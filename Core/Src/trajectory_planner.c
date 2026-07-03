#include "trajectory_planner.h"
#include <math.h>
static float _dt;

motion_status_t motion_init(motion_state_t *motion, float dt)
{
    return MOTION_OK;
}

motion_status_t motion_update(motion_state_t *motion, test_suite_t *ts)
{
    if(motion->is_running)
    {
        return MOTION_RUNNING;
    }

    float dx = motion->target_d - motion->d;
    TB6600_direction_t dir = dx > 0 ? TB6600_FORWARD : TB6600_REVERSE;

    float stop_dist = motion->v * motion->v / (2.0f * 50.0f);
    if(fabsf(dx) <= stop_dist)
    {
        motion->j_cmd = -dir * ts->j_max;
    }
    else
    {
        motion->j_cmd = dir * ts->j_max;
    }

    motion->a += motion->j_cmd * _dt;

    if(motion->a > ts->a_max)
    {
        motion->a = ts->a_max;
    }

    if(motion->a < -ts->a_max)
    {
        motion->a = - ts->a_max;
    }

    motion->v = motion->a * _dt;
    motion->d = motion->v * _dt;

    if(motion->v >= 0)
    {
        TB6600_set_direction(TB6600_FORWARD);
    }
    else
    {
        TB6600_set_direction(TB6600_REVERSE);
    }

    float freq = fabsf(motion->v * motion->steps_per_mm);

    TB6600_set_frequency(freq);

    if(fabsf(dx) < 0.01f && fabsf(motion->v) < 0.1f)
    {
        motion->is_running = false;
        TB6600_stop();
    }

    return MOTION_OK;
}
