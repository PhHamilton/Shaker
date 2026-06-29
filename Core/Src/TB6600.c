#include "TB6600.h"
#include "tim.h"

#define TB6600_TIMER_CLOCK_HZ 1000000UL
#define TB6600_MIN_SPEED 16UL
#define TB6600_MAX_SPEED 10000UL

static bool is_running = false;

TB6600_status_t TB6600_init(void)
{
    TB6600_disable();
    TB6600_set_direction(TB6600_FORWARD);

    return TB6600_OK;
}

TB6600_status_t TB6600_enable(void)
{
    HAL_GPIO_WritePin(TB6600_EN_GPIO_Port, TB6600_EN_Pin, GPIO_PIN_RESET);
    return TB6600_OK;
}

TB6600_status_t TB6600_disable(void)
{
    HAL_GPIO_WritePin(TB6600_EN_GPIO_Port, TB6600_EN_Pin, GPIO_PIN_SET);
    return TB6600_OK;
}

TB6600_status_t TB6600_set_direction(TB6600_direction_t dir)
{
    HAL_GPIO_WritePin(TB6600_DIR_GPIO_Port, TB6600_DIR_Pin, (GPIO_PinState)dir);
    return TB6600_OK;

}

TB6600_status_t TB6600_set_speed(uint32_t steps_per_sec)
{
    if(steps_per_sec < TB6600_MIN_SPEED || steps_per_sec > TB6600_MAX_SPEED)
    {
        return TB6600_ERROR;
    }

    uint32_t arr = TB6600_TIMER_CLOCK_HZ / steps_per_sec - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim14, arr);
    __HAL_TIM_SET_COMPARE(&htim14, TIM_CHANNEL_1, arr/2);

    return TB6600_OK;
}

TB6600_status_t TB6600_start(TB6600_direction_t dir, uint32_t steps_per_sec)
{
    if(is_running)
    {
        return TB6600_BUSY;
    }

    TB6600_set_direction(dir);

    if(TB6600_set_speed(steps_per_sec) != TB6600_OK)
    {
        return TB6600_ERROR;
    }

    TB6600_enable();

    HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
    is_running = true;

    return TB6600_OK;
}

TB6600_status_t TB6600_stop(void)
{
    HAL_TIM_PWM_Stop(&htim14, TIM_CHANNEL_1);
    TB6600_disable();
    is_running = false;

    return TB6600_OK;
}
