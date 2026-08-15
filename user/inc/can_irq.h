#ifndef __CAN_IRQ_H
#define __CAN_IRQ_H
#include "main.h"
#include "can.h"
#include "protocol.h"


void CAN_Start(void);                       /* 启动CAN+开FIFO0接收中断 */
void CAN_Send(uint32_t ide, uint32_t id, uint8_t dlc, uint8_t *data);  /* ide=CAN_ID_STD/CAN_ID_EXT */
extern volatile uint8_t can_beep_cnt;       /* 蜂鸣器响次数 */
extern volatile int8_t  can_flow_cmd;       /* 流水灯状态 -1无, 0关, 1开 */

#endif
