#include "trajectory_planner.h"
#include <math.h>
static float _dt;

motion_status_t motion_init(motion_state_t *motion, float dt)
{
    if(motion == NULL || dt <= 0.0f)
    {
        return MOTION_ERROR;
    }

    _dt = dt;
    motion->j = 0.0f;
    motion->a = 0.0f;
    motion->v = 0.0f;
    motion->d = 0.0f;
    motion->j_cmd = 0.0f;
    motion->target_d = 0.0f;
    motion->is_running = false;

    return MOTION_OK;
}
motion_status_t motion_update(motion_state_t *motion, const test_suite_t *ts)
{
    if(motion == NULL || ts == NULL || _dt <= 0.0f)
    {
        return MOTION_OK;
    }

    if(!motion->is_running)
    {
        return MOTION_OK;
    }

    const float jmax = (float)ts->j_max;
    const float amax = (float)ts->a_max;
    const float vmax = (float)ts->v_max;

    float dx = motion->target_d - motion->d;
    float target_sign = (dx >= 0.0f) ? 1.0f : -1.0f;

    if((motion->v > 0.0f && dx < 0.0f) || (motion->v < 0.0f && dx > 0.0f))
    {
        motion->is_running = false;
        motion->v = 0.0f;
        motion->a = 0.0f;
        motion->d = motion->target_d;
        TB6600_stop();
        return MOTION_OK;
    }

    float speed = fabsf(motion->v);
    float stop_dist = (speed * speed) / (2.0f * amax);
    stop_dist += speed * (amax / jmax);

    bool toward_target = (motion->v * target_sign) >= 0.0f;
    bool need_brake = toward_target && (fabsf(dx) <= stop_dist);
    bool at_speed_limit = speed >= vmax;

    if(need_brake || at_speed_limit)
    {
        if(motion->a * target_sign > 0.0f)
        {
            motion->j_cmd = -target_sign * jmax;
        }
        else if(motion->a * target_sign < 0.0f && speed > 0.1f)
        {
            motion->j_cmd = 0.0f;
        }
        else
        {
            motion->j_cmd = target_sign * jmax;
        }
    }
    else
    {
        if(motion->a * target_sign < amax)
        {
            motion->j_cmd = target_sign * jmax;
        }
        else
        {
            motion->j_cmd = 0.0f;
        }
    }

    motion->d += motion->v * _dt
               + 0.5f * motion->a * _dt * _dt
               + (1.0f / 6.0f) * motion->j_cmd * _dt * _dt * _dt;

    motion->v += motion->a * _dt
               + 0.5f * motion->j_cmd * _dt * _dt;

    motion->a += motion->j_cmd * _dt;

    if(motion->a > amax) motion->a = amax;
    if(motion->a < -amax) motion->a = -amax;

    if(motion->v > vmax) motion->v = vmax;
    if(motion->v < -vmax) motion->v = -vmax;

    if(fabsf(dx) < 0.1f && fabsf(motion->v) < 0.1f || fabsf(motion->d - motion->target_d) < 0.01)
    {
        motion->is_running = false;
        motion->d = motion->target_d;
        motion->v = 0.0f;
        motion->a = 0.0f;
        TB6600_stop();
        return MOTION_OK;
    }

    TB6600_set_direction(
        (target_sign > 0.0f) ? TB6600_FORWARD : TB6600_REVERSE
    );

    TB6600_set_frequency(fabsf(motion->v) * motion->steps_per_mm);

    dx = motion->target_d - motion->d;

    return MOTION_RUNNING;
}
