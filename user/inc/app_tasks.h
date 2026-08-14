#ifndef __APP_TASKS_H
#define __APP_TASKS_H

/* 头文件调用 */
#include "cmsis_os2.h"
#include "protocol.h"

/* ============ 队列句柄(供中断/任务投递) ============ */
extern osMessageQueueId_t can_rx_queue;   /* CAN 接收消息队列 */
extern osMessageQueueId_t beep_queue;     /* 蜂鸣请求队列 */
#if BOARD_MASTER
extern osMessageQueueId_t vofa_cmd_queue; /* VOFA 指令队列(仅主板) */
#endif

void app_tasks_create(void);   /* 创建所有任务与队列 */

#endif /* __APP_TASKS_H */
