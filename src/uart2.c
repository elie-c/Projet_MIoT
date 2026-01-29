#include "uart2.h"
#include "main.h"      // pour Error_Handler
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart2;
static uint8_t rx_byte;

/**
  * @brief Initialisation USART2
  */
void UART2_Init(void)
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

    if (huart->Instance == USART2)
    {
        /*##-1- Enable peripherals and GPIO Clocks ##*/
        /* Enable GPIO TX/RX clock */
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* Enable USART2 clock */
        __HAL_RCC_USART2_CLK_ENABLE();

        /*##-2- Configure peripheral GPIO ####*/
        /* UART TX GPIO pin configuration  */
        GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_PULLUP;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF1_USART2;

        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /*##-3- Configure the NVIC for UART ##*/
        /* NVIC for USART1 */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

/**
  * @brief Envoi d’un caractère
  */
void UART2_SendChar(uint8_t c)
{
    HAL_UART_Transmit(&huart2, &c, 1, 100);
}

/**
  * @brief Envoi d’une chaîne
  */
void UART2_SendString(char *str)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 100);
}

void send_data(uint8_t *data, uint16_t size)
{
    HAL_UART_Transmit(&huart2, data, size, 100);
}


/**
  * @brief Démarrer réception en interruption
  */
void UART2_StartReceiveIT(void)
{
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
}

/**
  * @brief Callback réception UART
  */
#define RX_BUFFER_SIZE 32
static char rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_index = 0;

/* Simulated values for MPPT */
static uint16_t val_upan = 3200; // : 32.00 V
static uint16_t val_ubat = 1250; // : 12.50 V
static uint16_t val_cpan = 150;  // : 1.50 A
static uint16_t val_cbat = 200;  // : 2.00 A
static uint8_t val_soc = 85;     // : 85%

void UART2_SendNextData(void) {
    static uint8_t index = 0;
    char tx_buffer[32];

    switch (index) {
        case 0:
            sprintf(tx_buffer, "upan:%04d\n", val_upan);
            break;
        case 1:
            sprintf(tx_buffer, "ubat:%04d\n", val_ubat);
            break;
        case 2:
            sprintf(tx_buffer, "cpan:%04d\n", val_cpan);
            break;
        case 3:
            sprintf(tx_buffer, "cbat:%04d\n", val_cbat);
            break;
        case 4:
            sprintf(tx_buffer, "soc:%02d\n", val_soc);
            break;
        default:
            index = 0;
            return;
    }
    
    UART2_SendString(tx_buffer);
    
    index++;
    if (index > 4) {
        index = 0;
    }
}

void ProcessCommand(char *cmd) {
    char tx_buffer[32];

    if (strstr(cmd, "uPan") != NULL) {
        sprintf(tx_buffer, "upan:%04d\r\n", val_upan);
        UART2_SendString(tx_buffer);
    } else if (strstr(cmd, "ubat") != NULL) {
        sprintf(tx_buffer, "ubat:%04d\r\n", val_ubat);
        UART2_SendString(tx_buffer);
    } else if (strstr(cmd, "cpan") != NULL) {
        sprintf(tx_buffer, "cpan:%04d\r\n", val_cpan);
        UART2_SendString(tx_buffer);
    } else if (strstr(cmd, "cbat") != NULL) {
        sprintf(tx_buffer, "cbat:%04d\r\n", val_cbat);
        UART2_SendString(tx_buffer);
    } else if (strstr(cmd, "soc") != NULL) {
        sprintf(tx_buffer, "soc:%02d\r\n", val_soc);
        UART2_SendString(tx_buffer);
    } else {
        // Unknown command or just echo
        // UART2_SendString("Unknown Command\r\n");
    }
}

/**
  * @brief Callback réception UART
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
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

        // Redémarrer la réception
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}
    

