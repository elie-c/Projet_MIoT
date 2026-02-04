#ifndef ADC_H
#define ADC_H

#include "stm32f0xx_hal.h"

void ADC1_Init(void);
uint16_t ADC_ReadChannel(uint32_t channel);
void ADC_Update_MPPT_Values(void);

#endif // ADC_H
