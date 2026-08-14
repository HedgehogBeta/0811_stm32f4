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
static uint32_t Breath_Period_Ms = BREATH_PERIOD_MS;
static uint8_t Breath_Led_State = 0;

static uint32_t get_breath_led_period(void)
{
    return Breath_Period_Ms;
}

void update_breath_led_control(uint8_t new_control)
{
    Breath_Led_State = new_control;
}

void breath_led_control(void)
{
    switch(Breath_Led_State)
    {
        case 0:
            breath_led_off();
            break;
        case 1:
            breath_led_update();
            break;
    }
}

uint32_t update_breath_led_period(uint32_t new_breath_led_period)
{
    if(new_breath_led_period<=BREATH_PERIOD_MS)
    Breath_Period_Ms = new_breath_led_period;
    else
    {   
        Breath_Period_Ms=BREATH_PERIOD_MS;
    }
}

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
    uint32_t period = get_breath_led_period();
    uint32_t phase = HAL_GetTick() % period;
    uint32_t duty;

    if (phase <= (period / 2u)) {
        duty = (phase * 2u * PWM_MAX) / period;
    } else {
        duty = ((period - phase) * 2u * PWM_MAX) / period;
    }

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, duty);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty);
}

void breath_led_off(void)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0u);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0u);
}