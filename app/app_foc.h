/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.h
 * @Author       : Codex
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17 16:48:00
 * @Brief        : FOC 应用封装入口
 */

#ifndef __APP_FOC_H__
#define __APP_FOC_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "../board/armfly-v7/bsp/motor/foc_port.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
typedef struct {
    /* 应用层当前持有一份板级 FOC 端口对象 */
    foc_port_t port;
} app_foc_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化 FOC 应用对象
 * @param  app: 应用对象
 * @return true=初始化成功, false=初始化失败
 */
bool app_foc_init(app_foc_t *app);

/**
 * @brief  获取应用层持有的 FOC 控制器对象
 * @param  app: 应用对象
 * @return pmsm_foc_t 指针
 */
pmsm_foc_t *app_foc_get_foc(app_foc_t *app);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
