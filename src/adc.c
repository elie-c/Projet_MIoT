#include "adc.h"
#include "mppt_data.h"

ADC_HandleTypeDef hadc1;

/*
 * PA0 -> ADC_IN0 (uPan)
 * PA5 -> ADC_IN5 (uBat)
 * PA2 -> ADC_IN2 (cPan - iPan)
 * PA7 -> ADC_IN7 (cBat - iBat)
 */

void ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        // Error_Handler();
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (adcHandle->Instance == ADC1) {
        /* ADC1 clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /**ADC GPIO Configuration
        PA0     ------> ADC_IN0
        PA2     ------> ADC_IN2
        PA5     ------> ADC_IN5
        PA6     ------> ADC_IN6
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle) {
    if (adcHandle->Instance == ADC1) {
        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();

        /**ADC GPIO Configuration
        PA0     ------> ADC_IN0
        PA2     ------> ADC_IN2
        PA5     ------> ADC_IN5
        PA 6     ------> ADC_IN6
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6);
    }
}

uint16_t ADC_ReadChannel(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

    // Clear previous channel selection
    hadc1.Instance->CHSELR = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        return 0;
    }

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    return val;
}

void ADC_Update_MPPT_Values(void) {
    // Read uPan (PA0 -> Channel 0)
    // Scale factor to be determined, raw value for now
    val_upan = ADC_ReadChannel(ADC_CHANNEL_0);

    // Read uBat (PA5 -> Channel 5)
    val_ubat = ADC_ReadChannel(ADC_CHANNEL_5);

    // Read cPan (PA2 -> Channel 2)
    val_cpan = ADC_ReadChannel(ADC_CHANNEL_2);

    // Read cBat (PA6 -> Channel 6)
    val_cbat = ADC_ReadChannel(ADC_CHANNEL_6);
    
    // Soc calculation (dummy logic for now)
    val_soc = (uint8_t)((val_ubat * 100) / 4095);
}
