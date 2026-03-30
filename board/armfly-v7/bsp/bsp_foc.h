/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : board/armfly-v7/bsp/bsp_foc.h
 * @Author       : Codex
 * @Date         : 2026-03-17 15:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 10:10:00
 * @Brief        : armfly-v7 FOC 端口封装
 */

#ifndef __BSP_FOC_H__
#define __BSP_FOC_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "device.h"
#include "foc_hal.h"
#include "foc_profile.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
struct foc_port_ctx {
    device_t *pwm_dev;
    device_t *current_dev;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化板级 FOC 端口资源
 * @return true=初始化成功, false=初始化失败
 */
bool foc_port_init(void);

/**
 * @brief  获取电机参数
 * @return motor_profile_t 指针
 */
const motor_profile_t *foc_port_get_motor_profile(void);

/**
 * @brief  获取功率板参数
 * @return power_stage_profile_t 指针
 */
const power_stage_profile_t *foc_port_get_power_stage_profile(void);

/**
 * @brief  获取传感器参数
 * @return sensor_profile_t 指针
 */
const sensor_profile_t *foc_port_get_sensor_profile(void);

/**
 * @brief  获取控制器配置
 * @return foc_ctrl_cfg_t 指针
 */
const foc_ctrl_cfg_t *foc_port_get_ctrl_cfg(void);

/**
 * @brief  获取板级 HAL 接口
 * @return foc_hal_ops_t 指针
 */
const foc_hal_ops_t *foc_port_get_hal_ops(void);

/**
 * @brief  获取 HAL 用户数据
 * @return 用户数据指针
 */
void *foc_port_get_hal_user_data(void);

/**
 * @brief  注册电流环 ADC 中断回调
 * @param  irq_handler: ADC DMA 中断回调
 * @return true=注册成功, false=注册失败
 */
bool foc_port_register_current_loop_irq(int32_t (*irq_handler)(uint32_t irq, void *args, uint32_t length));
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
