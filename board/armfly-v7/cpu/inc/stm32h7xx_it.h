/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : stm32h7xx_it.h
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Lu Xianfan
 * @LastEditTime : 2026-03-16 11:39:57
 * @Brief        : armfly-v7 STM32H7 interrupt declarations
 */

#ifndef __STM32H7XX_IT_H__
#define __STM32H7XX_IT_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "cpu.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
