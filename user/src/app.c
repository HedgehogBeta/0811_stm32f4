/**
  ******************************************************************************
  * @file    app.c
  * @brief   应用层:状态机
  *
  *  状态与转移:
  *    ST_OFF   初始,四灯全灭
  *    长按 OFF      -> ST_FLOW   流水灯(LED1/2)
  *    短按 ST_FLOW  -> ST_BREATH 呼吸灯(LED3/4)
  *    短按 ST_BREATH-> ST_FLOW   切回流水
  *    长按 (非 OFF) -> ST_OFF    回到全灭
  *   每次状态切换蜂鸣器响一声(不同状态响不同时长以作区分)
  ******************************************************************************
  */
#include "app.h"
#include "main.h"
#include "button.h"
#include "led_flow.h"
#include "breath_led.h"
#include "buzzer.h"

typedef enum {
    ST_OFF = 0,   /* 初始:四灯全灭 */
    ST_FLOW,      /* 流水灯(LED1/2) */
    ST_BREATH     /* 呼吸灯(LED3/4) */
} AppState_t;

static AppState_t s_state = ST_OFF;

static void app_enter_state(AppState_t new_state)
{
    s_state = new_state;

    switch (new_state) {
        case ST_OFF:
            flow_led_off();
            breath_led_off();
            buzzer_beep(120u);
            break;
        case ST_FLOW:
            breath_led_off();
            flow_led_init();
            buzzer_beep(200u);
            break;
        case ST_BREATH:
            flow_led_off();
            breath_led_init();
            buzzer_beep(100u);
            break;
        default:
            break;
    }
}

void app_init(void)
{
    button_init();
    flow_led_init();
    breath_led_init();
    buzzer_init();
    s_state = ST_OFF;   /* 上电直接进入初始状态,不响蜂鸣器 */
}

void app_run(void)
{
    button_update();              /* 检测松开,生成按键事件 */
    ButtonEvent_t ev = button_get_event();

    if (ev == BTN_EVENT_LONG) {
        /* 长按:开机(进入流水) / 关机(回到全灭) */
        if (s_state == ST_OFF) {
            app_enter_state(ST_FLOW);
        } else {
            app_enter_state(ST_OFF);
        }
    } else if (ev == BTN_EVENT_SHORT) {
        /* 短按:流水 <-> 呼吸 切换 */
        if (s_state == ST_FLOW) {
            app_enter_state(ST_BREATH);
        } else if (s_state == ST_BREATH) {
            app_enter_state(ST_FLOW);
        }
        /* 在 OFF 状态短按不做任何事 */
    }

    /* 运行当前状态的效果 */
    switch (s_state) {
        case ST_FLOW:
            flow_led_update();
            break;
        case ST_BREATH:
            breath_led_update();
            break;
        case ST_OFF:
        default:
            break;
    }

    buzzer_update();
}
