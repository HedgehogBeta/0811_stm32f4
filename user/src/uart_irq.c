#include "uart_irq.h"

#define HEADER 0xFF

uint8_t rx_buffer[100];
uint8_t tx_buffer[100];
volatile uint8_t beep_count = 0;

void UART_Start_Receive(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_buffer, sizeof(rx_buffer));
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance != USART1)
        return;

    if (rx_buffer[0] == HEADER)
    {
        uint8_t cnt = 0;
        for (int i = 1; i < Size; i++)
            if (rx_buffer[i] == 0x01)
                cnt++;
        beep_count = cnt;
        memcpy(tx_buffer, rx_buffer, Size);
        HAL_UART_Transmit(&huart1, tx_buffer, Size, 100);
    }
    
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

