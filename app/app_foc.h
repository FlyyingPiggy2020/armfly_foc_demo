/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.h
 * @Author       : Codex
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 18:20:00
 * @Brief        : FOC 应用层接口
 */

#ifndef __APP_FOC_H__
#define __APP_FOC_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdbool.h>
#include "message_bus.h"
#include "bsp_foc.h"
#include "pmsm_foc.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct {
    pmsm_foc_t foc;
    struct msgbus_node node;
    msgbus_service_t foc_service;
    bool is_started;
    foc_scalar_t speed_ref;
} app_foc_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化 FOC 应用层
 * @return true=成功, false=失败
 */
bool app_foc_init(void);

/**
 * @brief  切换电机启停状态
 * @note   标定进行中会忽略该命令
 */
void app_foc_motor_switch(void);

/**
 * @brief  增加速度给定
 * @note   标定进行中会忽略该命令
 */
void app_foc_speed_inc(void);

/**
 * @brief  减少速度给定
 * @note   标定进行中会忽略该命令
 */
void app_foc_speed_dec(void);

/**
 * @brief  获取全局 FOC 控制器对象
 * @return pmsm_foc_t 控制器指针
 */
pmsm_foc_t *app_foc_get_foc(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __APP_FOC_H__ */
