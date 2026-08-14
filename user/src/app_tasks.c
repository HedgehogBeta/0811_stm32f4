#include "app_tasks.h"
#include "protocol.h"
#include "can_irq.h"
#include "uart_irq.h"
#include "buzzer.h"
#include "led_flow.h"
#include "breath_led.h"
#include <string.h>
#include <stdio.h>

/* 队列句柄 */
osMessageQueueId_t can_rx_queue;
osMessageQueueId_t beep_queue;
#if BOARD_MASTER
osMessageQueueId_t vofa_cmd_queue;   /* 仅主板 */
#endif

/* 心跳: LED1/2 交替闪烁 */
static void heartbeat_task(void *arg)
{
    for (;;)
    {
        
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
    for (;;)
    {
        can_app_run();//蜂鸣器响报文指定次数
        osDelay(10);
    }
}

/* CAN接收处理(主从分支) */
static void can_rx_task(void *arg)
{
    for (;;)
    {
#if BOARD_MASTER
        /* 主板: 蜂鸣 + 转发float打波 */
#else
        /* 从板: 蜂鸣 + 收0x012控呼吸 */
        CAN_Start();
        breath_led_control();
        osDelay(10);
#endif
    }
}

/* CAN周期发送(主从分支) */
static void can_tx_task(void *arg)
{
    for (;;)
    {
#if BOARD_MASTER
        /* 主板: 每50ms发0x012 */
#else
        /* 从板: 每10ms发100Hz float */
#endif
    }
}

#if BOARD_MASTER
/* VOFA指令处理(仅主板) */
static void vofa_rx_task(void *arg)
{
    for (;;)
    {
    }
}
#endif

/* 创建任务与队列(在freertos.c调用) */
void app_tasks_create(void)
{
#if BOARD_MASTER
    /* 仅主板: vofa队列/任务 */
#endif

}
