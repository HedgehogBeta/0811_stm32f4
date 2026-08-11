/**
  ******************************************************************************
  * @file    button.h
  * @brief   按键输入(INPUT_1 = PC11,外部中断),区分短按/长按
  *
  * @note    作业要求:按键默认低电平、按下为高电平。
  *          CubeMX 里 PC11 应配置为:下拉 GPIO_PULLDOWN + 双边沿
  *          (GPIO_MODE_IT_RISING_FALLING),这样按下/松开都能触发中断。
  ******************************************************************************
  */
#ifndef __BUTTON_H
#define __BUTTON_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT,   /* 短按(按住时长 < 长按阈值) */
    BTN_EVENT_LONG     /* 长按(按住时长 >= 长按阈值) */
} ButtonEvent_t;

void button_init(void);
void button_update(void);            /* 主循环周期调用,检测松开生成事件 */
ButtonEvent_t button_get_event(void);  /* 读取并清除一次按键事件 */

#ifdef __cplusplus
}
#endif

#endif /* __BUTTON_H */
