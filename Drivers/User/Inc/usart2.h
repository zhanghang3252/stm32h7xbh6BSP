#ifndef __USART_H__
#define __USART_H__
#include "main.h"

extern UART_HandleTypeDef huart2;
void MX_USART2_UART_Init(void);
void send_usart(UART_HandleTypeDef huart2,uint8_t *pData);
#endif 

