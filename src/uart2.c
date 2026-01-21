#include "uart2.h"
#include "main.h"      // pour Error_Handler
#include <string.h>

UART_HandleTypeDef huart2;
static uint8_t rx_byte;

/**
  * @brief Initialisation USART2
  */
void UART1_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief Envoi d’un caractère
  */
void UART1_SendChar(uint8_t c)
{
    HAL_UART_Transmit(&huart2, &c, 1, 100);
}

/**
  * @brief Envoi d’une chaîne
  */
void UART1_SendString(char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

/**
  * @brief Démarrer réception en interruption
  */
void UART1_StartReceiveIT(void)
{
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

/**
  * @brief Callback réception UART
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        UART1_SendChar(rx_byte); // echo
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}
