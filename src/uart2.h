#ifndef UART2_H
#define UART2_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_uart.h"

#include <stdint.h>

extern UART_HandleTypeDef huart2;

void UART2_Init(void);
void UART2_SendChar(uint8_t c);
void UART2_SendString(char *str);
void UART2_StartReceiveIT(void);
void UART2_SendNextData(void);

#endif /* UART2_H */
