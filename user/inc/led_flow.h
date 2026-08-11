/**
  ******************************************************************************
  * @file    led_flow.h
  * @brief   流水灯驱动(LED1 = PA4, LED2 = PA5)
  ******************************************************************************
  */
#ifndef __LED_FLOW_H
#define __LED_FLOW_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void flow_led_init(void);    /* 初始:两灯全灭 */
void flow_led_update(void);  /* 主循环周期调用,按间隔翻转实现流水 */
void flow_led_off(void);     /* 两灯全灭 */

#ifdef __cplusplus
}
#endif

#endif /* __LED_FLOW_H */
