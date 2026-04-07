/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : bsp_foc.h
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-02 19:05:00
 * @Brief        : armfly-v7 FOC 板级聚合接口
 */

#ifndef __BSP_FOC_H__
#define __BSP_FOC_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "device.h"
#include "foc.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
struct foc_port_ctx {
    device_t *pwm_dev;
    device_t *current_dev;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
bool foc_port_init(void);
const struct foc_config *foc_port_get_config(void);
const struct foc_port *foc_port_get_port(void);
void *foc_port_get_port_ctx(void);
bool foc_port_register_current_loop_irq(int32_t (*irq_handler)(uint32_t irq, void *args, uint32_t length));
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
