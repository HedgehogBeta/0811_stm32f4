/**
  ******************************************************************************
  * @file    breath_led.c
  * @brief   呼吸灯驱动(LED3 = PA6/TIM3_CH1, LED4 = PA7/TIM3_CH2)
  *          
  *          
  ******************************************************************************
  */
#include "breath_led.h"
#include "tim.h"

#define BREATH_PERIOD_MS    2000u   /* 一个呼吸周期(暗->亮->暗) */
#define PWM_MAX             1000u   /* TIM3 计数周期 = ARR + 1 */

void breath_led_init(void)
{
    /* 启动两个 PWM 通道,初始占空比 0(灭) */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0u);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0u);
}

void breath_led_update(void)
{
    uint32_t phase = HAL_GetTick() % BREATH_PERIOD_MS;
    uint32_t duty;

    if (phase <= (BREATH_PERIOD_MS / 2u)) {
        duty = (phase * 2u * PWM_MAX) / BREATH_PERIOD_MS;
    } else {
        duty = ((BREATH_PERIOD_MS - phase) * 2u * PWM_MAX) / BREATH_PERIOD_MS;
    }

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty);
}

void breath_led_off(void)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0u);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0u);
}
