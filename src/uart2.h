#ifndef UART2_H
#define UART2_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_dma.h"
#include "stm32f0xx_hal_uart.h"

#include <stdint.h>

extern UART_HandleTypeDef huart2;

void UART1_Init(void);
void UART1_SendChar(uint8_t c);
void UART1_SendString(char *str);
void UART1_StartReceiveIT(void);

#endif /* UART2_H */
