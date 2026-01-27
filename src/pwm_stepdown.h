#ifndef PWM_STEPDOWN_H
#define PWM_STEPDOWN_H

#include "stm32f0xx_hal.h"

/* Timer 1 Channel 3 - PA10 (TIM1_CH3) */
#define STEPDOWN_PWM_PIN_H          GPIO_PIN_10
#define STEPDOWN_PWM_PORT_H         GPIOA

/* Timer 1 Channel 3N - PB1 (TIM1_CH3N) */
#define STEPDOWN_PWM_PIN_L          GPIO_PIN_1
#define STEPDOWN_PWM_PORT_L         GPIOB

#define STEPDOWN_PWM_CLK_ENABLE()   do { __HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_GPIOB_CLK_ENABLE(); } while(0)

#define STEPDOWN_TIMER_INSTANCE     TIM1
#define STEPDOWN_TIMER_CLK_ENABLE() __HAL_RCC_TIM1_CLK_ENABLE()
#define STEPDOWN_TIMER_CH3          TIM_CHANNEL_3
#define STEPDOWN_DEAD_TIME          48  /* Temps mort: ~1µs (pour 48 MHz) */

extern TIM_HandleTypeDef htim1;

/**
 * @brief Initialiser le PWM dual (complémentaire) pour le step-down
 */
void PWM_StepDown_Init(void);

/**
 * @brief Définir le duty cycle du PWM (0-100%)
 * @param duty: Pourcentage (0 à 100)
 */
void PWM_StepDown_SetDuty(uint8_t duty);

/**
 * @brief Obtenir le duty cycle actuel
 * @retval Duty cycle en pourcentage
 */
uint8_t PWM_StepDown_GetDuty(void);

/**
 * @brief Démarrer le PWM
 */
void PWM_StepDown_Start(void);

/**
 * @brief Arrêter le PWM
 */
void PWM_StepDown_Stop(void);

#endif /* PWM_STEPDOWN_H */
