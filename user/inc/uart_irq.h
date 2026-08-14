#ifndef __UART_IRQ_H
#define __UART_IRQ_H

#include "main.h"
#include "usart.h"
#include <string.h>
#include "breath_led.h"

void UART_Start_Receive(void);
extern volatile uint8_t beep_count;

#endif