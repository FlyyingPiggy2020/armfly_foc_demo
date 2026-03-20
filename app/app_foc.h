/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.h
 * @Author       : Codex
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 10:10:00
 * @Brief        : FOC 应用封装入口
 */

#ifndef __APP_FOC_H__
#define __APP_FOC_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "device.h"
#include "foc_port.h"
#include "pmsm_foc.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct {
    pmsm_foc_t foc;
    device_t *pwm_dev;
    device_t *current_dev;
} app_foc_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化 FOC 应用对象
 * @return true=初始化成功, false=初始化失败
 */
bool app_foc_init(void);

/**
 * @brief  获取应用层持有的 FOC 控制器对象
 * @return pmsm_foc_t 指针
 */
pmsm_foc_t *app_foc_get_foc(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
