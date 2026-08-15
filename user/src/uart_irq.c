#include "uart_irq.h"
#include "uart_irq.h"
#include "app_tasks.h" /* vofa_cmd_queue */
#include "protocol.h"
#include <string.h>

uint8_t rx_buffer[16];

void UART_Start_Receive(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, sizeof(rx_buffer));
}

/*justfloat打波参数*/
void UART_Send_Float(float val)
{
    uint8_t frame[8];
    memcpy(&frame[0], &val, 4);
    frame[4] = 0x00;
    frame[5] = 0x00; /* 帧尾 00 00 80 7F */
    frame[6] = 0x80;
    frame[7] = 0x7F;
    HAL_UART_Transmit(&huart1, frame, sizeof(frame), 100);
}

void UART_Send_Log(const char *s, uint16_t len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)s, len, 100);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance != USART1)
        return;

#if BOARD_MASTER    
    if (Size >= 5u && rx_buffer[0] == 0xA5u && rx_buffer[4] == 0x5Au) // 呼吸灯指令
    {
        BreathCtrl_t ctrl;
        ctrl.onoff = rx_buffer[1];
        ctrl.period_code = rx_buffer[2];
        osMessageQueuePut(vofa_cmd_queue, &ctrl, 0, 0); /* 中断投递,任务处理 */
    }
#endif
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, sizeof(rx_buffer));
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1)
        return;
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, sizeof(rx_buffer));
}
