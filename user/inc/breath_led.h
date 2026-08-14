/**
  ******************************************************************************
  * @file    breath_led.h
  * @brief   呼吸灯驱动(LED3 = PA6/TIM3_CH1, LED4 = PA7/TIM3_CH2)
  *          用 TIM3 PWM 占空比三角波实现呼吸
  ******************************************************************************
  */
#ifndef __BREATH_LED_H
#define __BREATH_LED_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void breath_led_init(void);   /* 启动 PWM,占空比 0(灭) */
void breath_led_update(void); /* 主循环周期调用,更新占空比实现呼吸 */
void breath_led_off(void);    /* 占空比 0(灭) */
void update_breath_led_control(uint8_t new_control);
void breath_led_control(void);

uint32_t update_breath_led_period(uint32_t new_breath_led_period);

#ifdef __cplusplus
}
#endif

#endif /* __BREATH_LED_H */
