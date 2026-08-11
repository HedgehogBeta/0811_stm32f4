/**
  ******************************************************************************
  * @file    button.c
  * @brief   按键输入(INPUT_1 = PC11,外部中断),区分短按/长按
  *
  *
  ******************************************************************************
  */
#include "button.h"

#define BTN_PRESSED_LEVEL   GPIO_PIN_SET   /* 按下为高电平 */
#define BTN_DEBOUNCE_MS     30u            /* 机械抖动消抖 */
#define BTN_LONG_PRESS_MS   800u           /* 长按判定阈值 */

static volatile uint32_t    s_press_tick = 0;   /* 按下时刻,0=未按下 */
static volatile ButtonEvent_t s_event = BTN_EVENT_NONE;
static uint32_t             s_last_edge = 0;    /* 边沿时刻(消抖) */
static uint8_t              s_rel_cand = 0;     /* 松开候选:1 表示已看到一次松开 */
static uint32_t             s_rel_cand_tick = 0;

void button_init(void)
{
    s_press_tick = 0;
    s_event = BTN_EVENT_NONE;
    s_last_edge = 0;
    s_rel_cand = 0;
    s_rel_cand_tick = 0;
}

ButtonEvent_t button_get_event(void)
{
    ButtonEvent_t ev = s_event;
    s_event = BTN_EVENT_NONE;
    return ev;
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin != INPUT_1_Pin) {
        return;
    }

    uint32_t now = HAL_GetTick();

    /* 消抖:相邻边沿间隔太短则忽略 */
    if ((now - s_last_edge) < BTN_DEBOUNCE_MS) {
        return;
    }
    s_last_edge = now;

    if (HAL_GPIO_ReadPin(INPUT_1_GPIO_Port, INPUT_1_Pin) == BTN_PRESSED_LEVEL) {
        s_press_tick = now;              /* 按下:记录时间 */
    }
}

/* 主循环周期调用:检测松开,按按住时长生成短按/长按事件 */
void button_update(void)
{
    if (s_press_tick == 0u) {
        return;
    }

    if (HAL_GPIO_ReadPin(INPUT_1_GPIO_Port, INPUT_1_Pin) == BTN_PRESSED_LEVEL) {
        s_rel_cand = 0;                  /* 仍处于按下,重置松开候选 */
        return;
    }

    /* 读到松开电平:要求稳定持续 BTN_DEBOUNCE_MS 再确认 */
    if (s_rel_cand == 0u) {
        s_rel_cand = 1;
        s_rel_cand_tick = HAL_GetTick();
    } else if ((uint32_t)(HAL_GetTick() - s_rel_cand_tick) >= BTN_DEBOUNCE_MS) {
        uint32_t dur = s_rel_cand_tick - s_press_tick;
        s_press_tick = 0;
        s_rel_cand = 0;

        if (dur >= BTN_LONG_PRESS_MS) {
            s_event = BTN_EVENT_LONG;
        } else if (dur >= BTN_DEBOUNCE_MS) {
            s_event = BTN_EVENT_SHORT;
        }
    }
}
