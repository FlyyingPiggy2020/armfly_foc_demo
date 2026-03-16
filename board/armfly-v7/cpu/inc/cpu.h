/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : cpu.h
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Lu Xianfan
 * @LastEditTime : 2026-03-16 11:39:57
 * @Brief        : armfly-v7 CPU abstraction interface
 */

#ifndef __CPU_H__
#define __CPU_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
/*---------- macro ----------*/
#define SYS_MODEL "armfly_foc_demo"
#define SYS_DATE  "1970-01-01"
#define SYS_VER   "S0.00.01"
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
void cpu_config(void);
void cpu_reset(void);
void cpu_get_uuid(uint8_t *pbuf, uint8_t len);
void udelay(uint32_t us);
void mdelay(uint32_t ms);
void tick_inc(void);
uint64_t tick_get(void);
uint64_t tick_get_from_isr(void);
void disable_irq(void);
void enable_irq(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
