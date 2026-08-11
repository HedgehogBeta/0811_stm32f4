/**
  ******************************************************************************
  * @file    led_flow.c
  * @brief   流水灯驱动(LED1 = PA4, LED2 = PA5),两灯交替闪烁模拟流水
  ******************************************************************************
  */
#include "led_flow.h"

#define FLOW_INTERVAL_MS   300u   /* 流水切换间隔 */

static uint32_t s_last_tick = 0;
static uint8_t  s_idx = 0;        /* 0:亮 LED1;1:亮 LED2 */

void flow_led_init(void)
{
    HAL_GPIO_WritePin(GPIOA, LED_1_Pin | LED_2_Pin, GPIO_PIN_RESET);
    s_last_tick = HAL_GetTick();
    s_idx = 0;
}

void flow_led_update(void)
{
    if ((uint32_t)(HAL_GetTick() - s_last_tick) < FLOW_INTERVAL_MS) {
        return;
    }
    s_last_tick = HAL_GetTick();

    if (s_idx == 0u) {
        HAL_GPIO_WritePin(GPIOA, LED_1_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, LED_2_Pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(GPIOA, LED_1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, LED_2_Pin, GPIO_PIN_SET);
    }
    s_idx ^= 1u;
}

void flow_led_off(void)
{
    HAL_GPIO_WritePin(GPIOA, LED_1_Pin | LED_2_Pin, GPIO_PIN_RESET);
}
