/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : board/armfly-v7/bsp/bsp_pwm.c
 * @Author       : Codex
 * @Date         : 2026-03-18 13:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-19 14:33:33
 * @Brief        : armfly-v7 电机 PWM 设备实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "device.h"
#include "foc_options.h"
#include "pwmc.h"
#include "stdbool.h"
#include "stm32h7xx_hal.h"

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static bool _motor_pwm_init(void);
static int32_t _motor_pwm_update_crr(uint8_t channel, uint32_t crr);
/*---------- function prototype ----------*/

static void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/*---------- variable ----------*/
TIM_HandleTypeDef htim1;

static pwmc_describe_t s_motor_pwm_desc = {
    .is_enable = false,
    .is_manual_freq = true,
    .priv = {
        .arr = 12500,
    },
    .priv.channel = {
        [PWMC_CHANNEL1] = { .used = true, .duty = 0.5f, .crr = 0U },
        [PWMC_CHANNEL2] = { .used = true, .duty = 0.5f, .crr = 0U },
        [PWMC_CHANNEL3] = { .used = true, .duty = 0.5f, .crr = 0U },
    },
    .ops = {
        .init = _motor_pwm_init,
        .update_crr = _motor_pwm_update_crr,
    },
};

DEVICE_DEFINED(motor_pwm, pwmc, &s_motor_pwm_desc);
/*---------- function ----------*/
static bool _motor_pwm_init(void)
{
    /* 1.采样测试GPIO初始化 */
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOK_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_7, GPIO_PIN_RESET);

    /*Configure GPIO pin : PJ7 */
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

    /* 2.DMA使能，定时器触发ADC采样用 */
    __HAL_RCC_DMA1_CLK_ENABLE();

    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    TIM_MasterConfigTypeDef sMasterConfig = { 0 };
    TIM_OC_InitTypeDef sConfigOC = { 0 };
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = { 0 };

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
    htim1.Init.Period = 12500;
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim1) != HAL_OK) {
        return false;
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC4REF;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK) {
        return false;
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM2;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        return false;
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        return false;
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
        return false;
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) {
        return false;
    }
    sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 255;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter = 0;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK) {
        return false;
    }

    HAL_TIM_MspPostInit(&htim1);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 6250); /* 50%占空比 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 6250); /* 50%占空比 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 6250); /* 50%占空比 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 300);  /* TRGO触发点 */

    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
        return false;
    }
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
        return false;
    }
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) {
        return false;
    }
    if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4) != HAL_OK) {
        return false;
    }

    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1) != HAL_OK) {
        return false;
    }
    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2) != HAL_OK) {
        return false;
    }
    if (HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3) != HAL_OK) {
        return false;
    }
    return true;
}

static int32_t _motor_pwm_update_crr(uint8_t channel, uint32_t crr)
{
    if (channel >= IOCTL_CONFIG_PWMC_CHANNEL) {
        return E_WRONG_ARGS;
    }

    switch (channel) {
        case PWMC_CHANNEL1:
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, crr);
            break;
        case PWMC_CHANNEL2:
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, crr);
            break;
        case PWMC_CHANNEL3:
            __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, crr);
            break;
        default:
            return E_WRONG_ARGS;
    }

    return E_OK;
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim_pwm)
{
    if (htim_pwm->Instance == TIM1) {
        /* Peripheral clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
}

static void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    if (htim->Instance == TIM1) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOK_CLK_ENABLE();
        __HAL_RCC_GPIOJ_CLK_ENABLE();
        /**TIM1 GPIO Configuration
        PA8     ------> TIM1_CH1
        PK0     ------> TIM1_CH1N
        PJ11     ------> TIM1_CH2
        PJ10     ------> TIM1_CH2N
        PJ9     ------> TIM1_CH3
        PJ8     ------> TIM1_CH3N
        */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);
    }
}
/*---------- end of file ----------*/
