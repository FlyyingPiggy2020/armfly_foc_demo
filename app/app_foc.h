/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.h
 * @Author       : lxf
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-02 18:40:00
 * @Brief        : FOC 应用层接口
 */

#ifndef __APP_FOC_H__
#define __APP_FOC_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdbool.h>
#include "foc.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化 FOC 应用层
 * @return true=成功, false=失败
 */
bool app_foc_init(void);

/**
 * @brief  运行 FOC 慢环任务
 */
void app_foc_process(void);

/**
 * @brief  切换电机启停状态
 * @note   对齐进行中会忽略该命令
 */
void app_foc_motor_switch(void);

/**
 * @brief  增加速度给定
 * @note   对齐进行中会忽略该命令
 */
void app_foc_speed_inc(void);

/**
 * @brief  减少速度给定
 * @note   对齐进行中会忽略该命令
 */
void app_foc_speed_dec(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __APP_FOC_H__ */
