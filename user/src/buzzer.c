/**
  ******************************************************************************
  * @file    buzzer.c
  * @brief   蜂鸣器驱动(BEEP = PA8,高电平响),非阻塞
  ******************************************************************************
  */
#include "buzzer.h"

static volatile uint32_t s_beep_end = 0;   /* 0 表示当前不响 */

void buzzer_init(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
    s_beep_end = 0;
}

void buzzer_beep(uint32_t dur_ms)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET);
    s_beep_end = HAL_GetTick() + dur_ms;
}

void buzzer_off(void)
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET);
    s_beep_end = 0;
}

void buzzer_update(void)
{
    if ((s_beep_end != 0u) && ((int32_t)(HAL_GetTick() - s_beep_end) >= 0)) {
        buzzer_off();
    }
}
