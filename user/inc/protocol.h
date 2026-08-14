#ifndef __PROTOCOL_H
#define __PROTOCOL_H

/* ============================================================
 * 协议与公共定义
 *
 * ============================================================ */

/* ============ 配置切换 ============ */
#ifndef BOARD_MASTER
#define BOARD_MASTER  0    /* 1=主板, 0=从板 */
#endif

#include "main.h"
#define CAN_ID_MASTER_CTRL    0x012U
#define CAN_ID_SLAVE_FEEDBACK 0x02010101U
#define CAN_BEEP_ID_CMD       0x01020101U

//#define UART_BREATH_HEADER 0xee

typedef struct {
    uint8_t onoff;        /* 0=关 1=开 */
    uint8_t period_code;  /* 周期码 */
} BreathCtrl_t;

/* 从 CAN 中断投递到任务队列的一条消息 */
typedef struct {
    uint8_t  ide; 
    uint32_t id;          
    uint8_t  dlc;
    uint8_t  data[8];
} CanMsg_t;


#endif /* __PROTOCOL_H */
