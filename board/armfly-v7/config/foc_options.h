/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_options.h
 * @Author       : lxf
 * @Date         : 2026-03-18 13:36:34
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-18 14:14:28
 * @Brief        :
 */
#ifndef __FOC_OPTIONS_H__
#define __FOC_OPTIONS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
/*---------- macro ----------*/
#define FOC_PWM_TIMER_CLK_HZ (200000000UL)                            // 定时器时钟频率 72MHz
#define FOC_PWM_FREQ_HZ      (8000UL)                                 // 开关频率 8HZ
#define FOC_PWM_ARR          (FOC_PWM_TIMER_CLK_HZ / FOC_PWM_FREQ_HZ) // 定时器自动重装载值(默认预分频为0)
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif

#endif /* __FOC_OPTIONS_H__ */
