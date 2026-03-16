/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : cpu.c
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Lu Xianfan
 * @LastEditTime : 2026-03-16 11:39:57
 * @Brief        : armfly-v7 CPU abstraction implementation
 */

/*---------- includes ----------*/
#include "cpu.h"
#include "flash_map.h"
#include "stm32h7xx.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static volatile uint64_t s_ticks = 0;
static volatile uint32_t s_critical_depth = 0;
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
void cpu_config(void)
{
}

void cpu_reset(void)
{
    NVIC_SystemReset();
}

void cpu_get_uuid(uint8_t *pbuf, uint8_t len)
{
    uint8_t copy_len = len;
    const uint8_t *uuid_ptr = (const uint8_t *)(uintptr_t)CONFIG_CHIP_UUID_BASE;

    if (copy_len > CONFIG_CHIP_UUID_SIZE) {
        copy_len = CONFIG_CHIP_UUID_SIZE;
    }
    if (pbuf != NULL) {
        memcpy(pbuf, uuid_ptr, copy_len);
    }
}

void udelay(uint32_t us)
{
    volatile uint32_t index = us * 32U;

    while (index--) {
        __NOP();
    }
}

void mdelay(uint32_t ms)
{
    while (ms--) {
        udelay(1000U);
    }
}

void tick_inc(void)
{
    s_ticks++;
}

uint64_t tick_get(void)
{
    return s_ticks;
}

uint64_t tick_get_from_isr(void)
{
    return s_ticks;
}

void disable_irq(void)
{
    __disable_irq();
    s_critical_depth++;
}

void enable_irq(void)
{
    if (s_critical_depth > 0U) {
        s_critical_depth--;
        if (s_critical_depth == 0U) {
            __enable_irq();
        }
    }
}
/*---------- end of file ----------*/
