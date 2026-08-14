#ifndef __PROTOCOL_H
#define __PROTOCOL_H

/* ============================================================
 * 协议与公共定义
 *
 * ============================================================ */

/* ============ 配置切换 ============ */
#ifndef BOARD_MASTER
#define BOARD_MASTER  1    /* 1=主板, 0=从板 */
#endif

#define CAN_ID_MASTER_CTRL    0x012U
#define CAN_ID_SLAVE_FEEDBACK 0x02010101U


#endif /* __PROTOCOL_H */
