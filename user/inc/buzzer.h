/**
  ******************************************************************************
  * @file    buzzer.h
  * @brief   蜂鸣器驱动(BEEP = PA8,高电平响)
  ******************************************************************************
  */
#ifndef __BUZZER_H
#define __BUZZER_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void buzzer_init(void);              /* 初始化:不响 */
void buzzer_beep(uint32_t dur_ms);   /* 响 dur_ms 毫秒后自动关(非阻塞) */
void buzzer_off(void);               /* 立即关 */
void buzzer_update(void);            /* 主循环周期调用,维护非阻塞蜂鸣计时 */

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H */
