/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : board_options.h
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Lu Xianfan
 * @LastEditTime : 2026-03-16 11:39:57
 * @Brief        : armfly-v7 board options
 */

#ifndef __BOARD_OPTIONS_H__
#define __BOARD_OPTIONS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "cpu.h"
/*---------- macro ----------*/
#define __delay_ms(ms)              (mdelay(ms))
#define __delay_us(us)              (udelay(us))
#define __get_ticks()               (tick_get())
#define __get_ticks_from_isr()      (tick_get_from_isr())
#define __reset_system()            cpu_reset()
#define __enter_critical()          disable_irq()
#define __exit_critical()           enable_irq()
#define __enter_critical_from_isr() disable_irq()
#define __exit_critical_from_isr()  enable_irq()
#define __ticks2ms(ticks)           (ticks)
#define __ms2ticks(ms)              (ms)

#ifndef ASSERT
#define ASSERT(expr)                                      \
    do {                                                  \
        if (!(expr)) {                                    \
            xlog_cout("ASSERT in %s:%d\r\n", __FILE__, __LINE__); \
            while (1) {                                   \
            }                                             \
        }                                                 \
    } while (0)
#endif

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <h> FP-SDK功能配置
//
//   <e> 日志功能是否开启
#define CONFIG_FPLOG                1
//   </e>
//
//   <e> 动态内存管理
//   <o> 动态内存池大小
#define CONFIG_HEAP_TOTAL_SIZE      4096
//   </e>
//
// </h>
//------------- <<< end of configuration section >>> -----------------------
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
