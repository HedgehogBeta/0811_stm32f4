#include "app_tasks.h"
#include "protocol.h"
#include "can_irq.h"
#include "uart_irq.h"
#include "buzzer.h"
#include "led_flow.h"
#include "breath_led.h"
#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct 
{
    TickType_t last_wake_time;
    TickType_t period;
     /* data */
}PeriodicControl_t;


static void PeriodicControl_Init(PeriodicControl_t *control, uint32_t period_ms)
{
    control->last_wake_time = xTaskGetTickCount();
    control->period = pdMS_TO_TICKS(period_ms);
}

static void PeriodicControl_Run(PeriodicControl_t *control)
{
    vTaskDelayUntil(&control->last_wake_time, control->period);
}

/* 队列句柄 */
osMessageQueueId_t can_rx_queue;
osMessageQueueId_t beep_queue;
#if BOARD_MASTER
osMessageQueueId_t vofa_cmd_queue; /* 仅主板 */
#endif

static const osThreadAttr_t s_heartbeat_attr = {.name = "heartbeat", .stack_size = 256, .priority = osPriorityNormal};
static const osThreadAttr_t s_breath_attr = {.name = "breath", .stack_size = 256, .priority = osPriorityNormal};
static const osThreadAttr_t s_canrx_attr = {.name = "can_rx", .stack_size = 256, .priority = osPriorityNormal};
static const osThreadAttr_t s_cantx_attr = {.name = "can_tx", .stack_size = 256, .priority = osPriorityNormal};
static const osThreadAttr_t s_buzzer_attr = {.name = "buzzer", .stack_size = 256, .priority = osPriorityNormal};
#if BOARD_MASTER
static const osThreadAttr_t s_vofarx_attr = {.name = "vofa_rx", .stack_size = 384, .priority = osPriorityNormal};
#endif



/* 心跳: LED1/2 交替闪烁 */
static void heartbeat_task(void *arg)
{
    for (;;)
    {
        flow_led_update();
        osDelay(10);
    }
}

/* 呼吸灯周期刷新 */
static void breath_task(void *arg)
{
    for (;;)
    {
        breath_led_update();
        osDelay(10);
    }
}

/* 蜂鸣器定次数响应 */
static void buzzer_task(void *arg)
{
    uint8_t n;
    for (;;)
    {
        if (osMessageQueueGet(beep_queue, &n, NULL, osWaitForever) != osOK)
        {
            continue;
        }
        for (uint8_t i = 0; i < n; i++)
        {
            buzzer_beep(150u);
            osDelay(150u);
            buzzer_update();
            osDelay(150u);
        }
        buzzer_off();
    }
}

/* CAN接收处理(主从分支) */
static void can_rx_task(void *arg)
{
    CanMsg_t msg;

    for (;;)
    {
        if (osMessageQueueGet(can_rx_queue, &msg, NULL, osWaitForever) != osOK)
            continue;

        /* 蜂鸣命令: 主从板都响应 */
        if (msg.ide == CAN_ID_EXT && msg.id == CAN_BEEP_ID_CMD && msg.dlc >= 1)
        {
            uint8_t n = msg.data[0];
            osMessageQueuePut(beep_queue, &n, 0, 0); // 入队
        }
#if BOARD_MASTER
        /* 主板: 收到从板 100Hz float → 打波到 VOFA */
        else if (msg.ide == CAN_ID_EXT && msg.id == CAN_ID_SLAVE_FEEDBACK && msg.dlc >= 4) {
            float val;
            memcpy(&val, msg.data, 4);
            UART_Send_Float(val);
        }
#else
        /* 从板: 收到主板呼吸控制 → 更新本地呼吸灯 */
        else if (msg.ide == CAN_ID_STD && msg.id == CAN_ID_MASTER_CTRL && msg.dlc >= 2) {
            breath_led_set_enable(msg.data[0]);
            breath_led_set_period((uint16_t)msg.data[1] * 100u);
        }
#endif
    }
}

/* CAN周期发送(主从分支) */
static void can_tx_task(void *arg)
{
    PeriodicControl_t pct = {0};
    PeriodicControl_Init(&pct,10);//防周期性漂移计时10ms
    for (;;)
    {
        uint8_t d[4];
#if BOARD_MASTER
        /* 主板: 每50ms发0x012 */
        d[0] = get_breath_led_control();
        d[1] = get_breath_led_period();
        CAN_Send(CAN_ID_STD, CAN_ID_MASTER_CTRL, 2, d);
        osDelay(50);
#else
        /* 从板: 每10ms发100Hz float */
        float value = get_duty();
        uint8_t data[4];
        memcpy(data,&value,sizeof(value));
        CAN_Send(CAN_ID_SLAVE_FEEDBACK,sizeof(value),data);
        PeriodicControl_Run(&pct);

#endif
    }
}

#if BOARD_MASTER
/* VOFA指令处理(仅主板) */
static void vofa_rx_task(void *arg)
{
    BreathCtrl_t ctrl;
    char line[32];
    for (;;)
    {
        if (osMessageQueueGet(vofa_cmd_queue, &ctrl, NULL, osWaitForever) != osOK)
            continue;

        update_breath_led_control(ctrl.onoff);
        update_breath_led_period((uint16_t)ctrl.period_code * 100u);

        int len = snprintf(line, sizeof(line), "breath=%d period=%dms\r\n",
                           ctrl.onoff, (int)((uint16_t)ctrl.period_code * 100u));
        UART_Send_Log(line, (uint16_t)len);
    }
}
#endif

/* 创建任务与队列(在freertos.c调用) */
void app_tasks_create(void)
{
    can_rx_queue = osMessageQueueNew(8, sizeof(CanMsg_t), NULL);
    beep_queue = osMessageQueueNew(8, sizeof(uint8_t), NULL);
#if BOARD_MASTER
    vofa_cmd_queue = osMessageQueueNew(4, sizeof(BreathCtrl_t), NULL);
#endif

    osThreadNew(heartbeat_task, NULL, &s_heartbeat_attr);
    osThreadNew(breath_task, NULL, &s_breath_attr);
    osThreadNew(can_rx_task, NULL, &s_canrx_attr);
    osThreadNew(can_tx_task, NULL, &s_cantx_attr);
    osThreadNew(buzzer_task, NULL, &s_buzzer_attr);
#if BOARD_MASTER
    osThreadNew(vofa_rx_task, NULL, &s_vofarx_attr);
#endif
}
