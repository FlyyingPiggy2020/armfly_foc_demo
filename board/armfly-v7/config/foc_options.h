/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_options.h
 * @Author       : lxf
 * @Date         : 2026-03-18 13:36:34
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 10:53:54
 * @Brief        :
 */
#ifndef __FOC_OPTIONS_H__
#define __FOC_OPTIONS_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
/*---------- macro ----------*/
#define FOC_CURRENT_SENSE_MODE 2 /* 1=单电阻采样, 2=双电阻采样, 3=三电阻采样 */

#define FOC_
#define FOC_PWM_TIMER_CLK_HZ   (200000000UL)                            // 定时器时钟频率 200MHz
#define FOC_PWM_FREQ_HZ        (8000UL)                                 // 开关频率 8HZ
#define FOC_PWM_ARR            (FOC_PWM_TIMER_CLK_HZ / FOC_PWM_FREQ_HZ) // 定时器自动重装载值(默认预分频为0)
#define FOC_I_BASE             (1.0f)                                   // 电流基准值，单位安培，电流量程
#define FOC_V_BASE             (12.0f / 1.732f)                         // 电压基准值，单位伏特，额定电压
#define FOC_OMEGA_BASE         (2.0f * 3.1415926f *)                    // 角速度基准值，单位弧度每秒，对应于开关频率
#define FOC_ADC_FULL_SCALE     (65535U)                                 // ADC 满量程码值
#define FOC_ADC_REF_VOLTAGE    (3.3f)                                   // ADC 参考电压
#define FOC_CURRENT_SENSE_GAIN (100.0f)                                 // 电流采样增益
#define FOC_SHUNT_RESISTOR_OHM (0.01f)                                  // 分流电阻阻值，单位欧姆

#define FOC_I_PU_SCALE         (FOC_ADC_REF_VOLTAGE / (FOC_ADC_FULL_SCALE * FOC_CURRENT_SENSE_GAIN * FOC_SHUNT_RESISTOR_OHM * FOC_I_BASE))
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif

#endif /* __FOC_OPTIONS_H__ */
