#ifndef UART1_H
#define UART1_H

#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_uart.h"

#include <stdint.h>

extern UART_HandleTypeDef hUART1;

void UART1_Init(void);
void UART1_SendChar(uint8_t c);
void UART1_SendString(char *str);
void UART1_StartReceiveIT(void);
void UART1_SendNextData(void);

#endif /* UART1_H */
