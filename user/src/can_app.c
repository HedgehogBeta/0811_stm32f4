#include "can_app.h"
#include "can_irq.h"
#include "buzzer.h"
#include "led_flow.h"

typedef enum
{
    CAN_APP_IDLE,   /* 空闲状态 */
    CAN_APP_BEEPING /* 蜂鸣状态 */
} CanAppState_t;

static CanAppState_t s_state = CAN_APP_IDLE;
static uint8_t s_flow_on = 0; /* 流水灯开关状态 */

void can_app_init(void)
{
    s_state = CAN_APP_IDLE;
    s_flow_on = 0;
    flow_led_off();
}

void can_app_run(void)
{
    /* 状态判断 */
    if (can_flow_cmd >= 0)
    {
        s_flow_on = (can_flow_cmd != 0);
        can_flow_cmd = -1;
        if (s_flow_on)
            flow_led_init();
        else
            flow_led_off();
        uint8_t d[3] = {'O', 'K', s_flow_on}; /* 填OK+状态 */
        CAN_Send(CAN_ID_EXT, 0x02010201U, 3, d);
    }
    if (can_beep_cnt > 0 && s_state == CAN_APP_IDLE)
    {
        s_state = CAN_APP_BEEPING;
    }

    /* 状态执行 */
    if (s_state == CAN_APP_BEEPING)
    {
        uint8_t n = can_beep_cnt;
        can_beep_cnt = 0;
        for (uint8_t i = 0; i < n; i++)
        {
            buzzer_beep(150u);
            HAL_Delay(150u);
            buzzer_update();
            HAL_Delay(150u);
        }
        uint8_t d[2] = {'O', 'K'};
        CAN_Send(CAN_ID_EXT, 0x02010101U, 2, d);
        s_state = CAN_APP_IDLE;
    }

    /* 流水灯持续运行 */
    if (s_flow_on)
        flow_led_update();
}
