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
bool app_foc_init(void);
void app_foc_toggle_enable(void);
void app_foc_speed_up(void);
void app_foc_speed_down(void);
pmsm_foc_t *app_foc_get_foc(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __APP_FOC_H__ */
