#include "pwm_stepdown.h"
#include "main.h"

TIM_HandleTypeDef htim1;
static uint8_t current_duty = 25;  /* Duty cycle initial à 25% */

/**
 * @brief Initialiser le PWM dual (complémentaire) pour le step-down
 * Pin 1 (PA10): PWM normal
 * Pin 2 (PB1): PWM complémentaire (inversé)
 * Fréquence PWM: 48 MHz / (PSC+1) / (ARR+1)
 * Configuration: PSC=47, ARR=999 => fPWM = 48M / 48 / 1000 = 1 kHz
 */
void PWM_StepDown_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_OC_InitTypeDef sConfigOC;

    /* Enable GPIO clock */
    STEPDOWN_PWM_CLK_ENABLE();

    /* Enable Timer clock */
    STEPDOWN_TIMER_CLK_ENABLE();

    /* Configure GPIO pin H (PA10 - TIM1_CH3) */
    GPIO_InitStruct.Pin       = STEPDOWN_PWM_PIN_H;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;

    HAL_GPIO_Init(STEPDOWN_PWM_PORT_H, &GPIO_InitStruct);

    /* Configure GPIO pin L (PB1 - TIM1_CH3N) */
    GPIO_InitStruct.Pin       = STEPDOWN_PWM_PIN_L;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;

    HAL_GPIO_Init(STEPDOWN_PWM_PORT_L, &GPIO_InitStruct);

    /* Configure Timer */
    htim1.Instance           = STEPDOWN_TIMER_INSTANCE;
    htim1.Init.Prescaler     = 47;           /* 48 MHz / 48 = 1 MHz */
    htim1.Init.CounterMode   = TIM_COUNTERMODE_UP;
    htim1.Init.Period        = 999;          /* 1 MHz / 1000 = 1 kHz PWM */
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure dead time (temps mort) via registre BDTR */
    htim1.Instance->BDTR = (htim1.Instance->BDTR & ~TIM_BDTR_DTG) | STEPDOWN_DEAD_TIME;
    /* Activer les sorties du Timer 1 */
    htim1.Instance->BDTR |= TIM_BDTR_MOE;

    /* Configure PWM Channel 3 avec sortie complémentaire */
    sConfigOC.OCMode       = TIM_OCMODE_PWM1;
    sConfigOC.Pulse        = (999 * current_duty) / 100;  /* Duty cycle normal */
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity  = TIM_OCNPOLARITY_LOW;   /* Sortie complémentaire inversée */
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState  = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, STEPDOWN_TIMER_CH3) != HAL_OK)
    {
        Error_Handler();
    }

    /* Start PWM on Channel 3 and complementary output */
    HAL_TIM_PWM_Start(&htim1, STEPDOWN_TIMER_CH3);
    HAL_TIMEx_PWMN_Start(&htim1, STEPDOWN_TIMER_CH3);
}

/**
 * @brief Définir le duty cycle du PWM (0-100%)
 * @param duty: Pourcentage (0 à 100)
 */
void PWM_StepDown_SetDuty(uint8_t duty)
{
    if (duty > 100)
        duty = 100;

    current_duty = duty;

    /* Calculer la valeur du registre de comparaison pour Channel 3 */
    uint32_t pulse = (999 * duty) / 100;

    /* Mettre à jour le PWM Channel 3 (la sortie complémentaire se fait automatiquement) */
    __HAL_TIM_SET_COMPARE(&htim1, STEPDOWN_TIMER_CH3, pulse);
}

/**
 * @brief Obtenir le duty cycle actuel
 * @retval Duty cycle en pourcentage
 */
uint8_t PWM_StepDown_GetDuty(void)
{
    return current_duty;
}

/**
 * @brief Démarrer le PWM
 */
void PWM_StepDown_Start(void)
{
    HAL_TIM_PWM_Start(&htim1, STEPDOWN_TIMER_CH3);
    HAL_TIMEx_PWMN_Start(&htim1, STEPDOWN_TIMER_CH3);
}

/**
 * @brief Arrêter le PWM
 */
void PWM_StepDown_Stop(void)
{
    HAL_TIM_PWM_Stop(&htim1, STEPDOWN_TIMER_CH3);
    HAL_TIMEx_PWMN_Stop(&htim1, STEPDOWN_TIMER_CH3);
}
