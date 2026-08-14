#ifndef __UART_IRQ_H
#define __UART_IRQ_H

#include "main.h"
#include "usart.h"

void UART_Start_Receive(void);
void UART_Send_Float(float val);// 打波
void UART_Send_Log(const char *s, uint16_t len);//log输出参数

#endif