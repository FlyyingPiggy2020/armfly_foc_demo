/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_port.h
 * @Author       : Codex
 * @Date         : 2026-03-17 15:40:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17 16:15:00
 * @Brief        : armfly-v7 FOC 端口封装
 */

#ifndef __FOC_PORT_H__
#define __FOC_PORT_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "analog.h"
#include "device.h"
#include "options.h"
#include "pwmc.h"
#include "../../../../components/fp-sdk/motion/foc/pmsm_foc.h"
/*---------- macro ----------*/
#define FOC_PORT_PHASE_CURRENT_CHANNEL_COUNT 2U
/*---------- type define ----------*/
typedef struct {
    /* 板级端口持有一份完整的 FOC 实例以及底层设备句柄 */
    pmsm_foc_t foc;
    foc_hal_ops_t hal_ops;
    motor_profile_t motor_profile;
    power_stage_profile_t power_stage_profile;
    sensor_profile_t sensor_profile;
    foc_ctrl_cfg_t ctrl_cfg;
    device_t *phase_current_dev;
    device_t *bus_voltage_dev;
    device_t *pwm_dev;
    device_t *angle_dev;
} foc_port_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化板级 FOC 端口，并完成 pmsm_foc_init
 * @param  port: 端口对象
 * @return true=初始化成功, false=初始化失败
 */
bool foc_port_init(foc_port_t *port);

/**
 * @brief  释放端口实例持有的 FOC 资源和设备句柄
 * @param  port: 端口对象
 * @return 无
 */
void foc_port_deinit(foc_port_t *port);

/**
 * @brief  获取端口内持有的 FOC 控制器对象
 * @param  port: 端口对象
 * @return pmsm_foc_t 指针
 */
pmsm_foc_t *foc_port_get_foc(foc_port_t *port);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
