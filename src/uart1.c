#include "uart1.h"
#include "mppt_data.h"
#include "main.h"      // pour Error_Handler
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart1;
static uint8_t rx_byte;

/**
  * @brief Initialisation USART1
  */
void UART1_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief Initialize the UART MSP.
 * This function configures the hardware resources used in this example:
 *  - Peripheral's clock enable
 *  - Peripheral's GPIO Configuration
 *  - NVIC configuration for UART interrupt request enable
 * @param huart: UART handle pointer
 * @retval None
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (huart->Instance == USART1)
    {
        /*##-1- Enable peripherals and GPIO Clocks ##*/
        /* Enable GPIO TX/RX clock */
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* Enable USART1 clock */
        __HAL_RCC_USART1_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ####*/
        /* UART TX/RX GPIO pin configuration  */
        /* PA9 -> TX, PA10 -> RX */
        GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_USART1;

        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /*##-3- Configure the NVIC for UART ##*/
        /* NVIC for USART1 */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

/**
  * @brief Envoi d’un caractère
  */
void UART1_SendChar(uint8_t c)
{
    HAL_UART_Transmit(&huart1, &c, 1, 100);
}

/**
  * @brief Envoi d’une chaîne
  */
void UART1_SendString(char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 100);
}

void UART1_SendUint(uint16_t value, uint8_t digits)
{
    char c;
    uint16_t div = 1;

    // Calcul du diviseur (ex: 1000 pour 4 chiffres)
    for (uint8_t i = 1; i < digits; i++) {
        div *= 10;
    }

    for (uint8_t i = 0; i < digits; i++) {
        c = (value / div) + '0';
        HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, 100);
        value %= div;
        div /= 10;
    }
}



/**
  * @brief Démarrer réception en interruption
  */
void UART1_StartReceiveIT(void)
{
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

/**
  * @brief Callback réception UART
  */
#define RX_BUFFER_SIZE 32
static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_index = 0;

/* Global values for MPPT */
uint16_t val_upan = 0; 
uint16_t val_ubat = 0; 
uint16_t val_cpan = 0;  
uint16_t val_cbat = 0;  
uint8_t val_soc = 0;     

void UART1_SendNextData(void)
{
    static uint8_t index = 0;

    switch (index)
    {
        case 0:
            HAL_UART_Transmit(&huart1, (uint8_t *)"upan:", 5, 100);
            UART1_SendUint(val_upan, 4);
            HAL_UART_Transmit(&huart1, (uint8_t *)"\n", 1, 100);
            break;

        case 1:
            HAL_UART_Transmit(&huart1, (uint8_t *)"ubat:", 5, 100);
            UART1_SendUint(val_ubat, 4);
            HAL_UART_Transmit(&huart1, (uint8_t *)"\n", 1, 100);
            break;

        case 2:
            HAL_UART_Transmit(&huart1, (uint8_t *)"cpan:", 5, 100);
            UART1_SendUint(val_cpan, 4);
            HAL_UART_Transmit(&huart1, (uint8_t *)"\n", 1, 100);
            break;

        case 3:
            HAL_UART_Transmit(&huart1, (uint8_t *)"cbat:", 5, 100);
            UART1_SendUint(val_cbat, 4);
            HAL_UART_Transmit(&huart1, (uint8_t *)"\n", 1, 100);
            break;

        case 4:
            HAL_UART_Transmit(&huart1, (uint8_t *)"soc:", 4, 100);
            UART1_SendUint(val_soc, 2);
            HAL_UART_Transmit(&huart1, (uint8_t *)"\n", 1, 100);
            break;

        default:
            index = 0;
            return;
    
        index = 0;
    }
}



/**
  * @brief Callback réception UART
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /*
        // Store received char
        if (rx_index < RX_BUFFER_SIZE - 1) {
            if (rx_byte == '\r' || rx_byte == '\n') {
                rx_buffer[rx_index] = '\0'; // Terminate string
                if (rx_index > 0) {
                    ProcessCommand(rx_buffer);
                }
                rx_index = 0; // Reset buffer
            } else {
                rx_buffer[rx_index++] = rx_byte;
            }
        } else {
            // Buffer overflow, reset
            rx_index = 0;
        }
            */

        // Redémarrer la réception
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}
    

