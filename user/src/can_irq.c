#include "can_irq.h"
#include "breath_led.h"

volatile uint8_t can_beep_cnt = 0;
volatile int8_t can_flow_cmd = -1;

void CAN_Start(void)
{
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_Send(uint32_t ext_id, uint8_t dlc, uint8_t *data)
{
    CAN_TxHeaderTypeDef tx = {0};
    uint32_t mailbox = 0;
    tx.IDE = CAN_ID_EXT;
    tx.ExtId = ext_id;
    tx.DLC = dlc;
    tx.RTR = CAN_RTR_DATA;
    tx.TransmitGlobalTime = DISABLE;
    HAL_CAN_AddTxMessage(&hcan1, &tx, data, &mailbox);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance != CAN1)
        return;

    CAN_RxHeaderTypeDef rx = {0};
    uint8_t data[8] = {0};
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx, data) != HAL_OK)
        return;

}
