/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : stm32h7xx_it.c
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Lu Xianfan
 * @LastEditTime : 2026-03-16 11:39:57
 * @Brief        : armfly-v7 STM32H7 interrupt handlers
 */

/*---------- includes ----------*/
#include "stm32h7xx_it.h"
#include "stm32h7xx_hal.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

void MemManage_Handler(void)
{
    while (1) {
    }
}

void BusFault_Handler(void)
{
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
    tick_inc();
}
/*---------- end of file ----------*/
