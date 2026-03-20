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
#include "options.h"
#include "flash_map.h"
#include "stm32h7xx_hal.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$ER_IROM1$$Base[];
#elif defined(__GNUC__)
extern uint32_t _sisr_vector;
#else
#error "The compiler not armcc, armclang or gcc"
#endif
const char sys_Version[] __attribute__((section(".ARM.__at_0x08004000"))) = SYS_MODEL "-" SYS_DATE "-" SYS_VER "\0";
static uint8_t uuid[CONFIG_CHIP_UUID_SIZE] = { 0 };

static volatile uint64_t s_ticks = 0;
static volatile uint32_t s_critical_depth = 0;
/*---------- function prototype ----------*/
static void SystemClock_Config(void);
static void _cpu_error_handler(void);
/*---------- variable ----------*/
/*---------- function ----------*/
void cpu_config(void)
{
    uint32_t relocate = 0;

//    /* 1. 设置中断向量表 */
//#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
//    relocate = (uint32_t)Image$$ER_IROM1$$Base;
//#elif defined(__GNUC__)
//    relocate = (uint32_t)&_sisr_vector;
//#endif

//#ifdef VECT_TAB_SRAM
//    SCB->VTOR = SRAM_BASE | (relocate - SRAM_BASE);
//#else
//    SCB->VTOR = FLASH_BASE | (relocate - FLASH_BASE);
//#endif

    HAL_Init();
    
    /* 2. 配置时钟树 */
    SystemClock_Config();
    
    /* 3. 使能全局中断，退出临界段 */
    exit_critical();
    
    /* 6. 设置嘀嗒定时器为1Khz 内部会将SysTick_IRQn的优先级设置为15(最低) */
    SysTick_Config(SystemCoreClock / 1000UL);
    
    /* 7. 使能SEGGER RTT */
#if CONFIG_SEGGERRTT_ENABLE
    SEGGER_RTT_Init();
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#endif

    /* 8. 获取芯片唯一的UUID */
    memcpy(uuid, (const void *)CONFIG_CHIP_UUID_BASE, CONFIG_CHIP_UUID_SIZE);
    PRINT_BUFFER_CONTENT(COLOR_BLUE, "UUID:", uuid, sizeof(uuid));
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

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef rcc_osc_init_struct = { 0 };
    RCC_ClkInitTypeDef rcc_clk_init_struct = { 0 };

    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
    }

    rcc_osc_init_struct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    rcc_osc_init_struct.HSEState = RCC_HSE_ON;
    rcc_osc_init_struct.PLL.PLLState = RCC_PLL_ON;
    rcc_osc_init_struct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    rcc_osc_init_struct.PLL.PLLM = 5;
    rcc_osc_init_struct.PLL.PLLN = 160;
    rcc_osc_init_struct.PLL.PLLP = 2;
    rcc_osc_init_struct.PLL.PLLQ = 2;
    rcc_osc_init_struct.PLL.PLLR = 2;
    rcc_osc_init_struct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    rcc_osc_init_struct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    rcc_osc_init_struct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&rcc_osc_init_struct) != HAL_OK) {
        _cpu_error_handler();
    }

    rcc_clk_init_struct.ClockType =
        RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
        | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    rcc_clk_init_struct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    rcc_clk_init_struct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    rcc_clk_init_struct.AHBCLKDivider = RCC_HCLK_DIV2;
    rcc_clk_init_struct.APB3CLKDivider = RCC_APB3_DIV2;
    rcc_clk_init_struct.APB1CLKDivider = RCC_APB1_DIV2;
    rcc_clk_init_struct.APB2CLKDivider = RCC_APB2_DIV2;
    rcc_clk_init_struct.APB4CLKDivider = RCC_APB4_DIV2;
    if (HAL_RCC_ClockConfig(&rcc_clk_init_struct, FLASH_LATENCY_2) != HAL_OK) {
        _cpu_error_handler();
    }
}

static void _cpu_error_handler(void)
{
    __disable_irq();
    while (1) {
    }
}
/*---------- end of file ----------*/
