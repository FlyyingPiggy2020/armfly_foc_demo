/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : bsp_encoder.h
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-02 16:20:00
 * @Brief        : armfly-v7 板级磁编码器实时读取接口
 */

#ifndef __BSP_ENCODER_H__
#define __BSP_ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "foc_port.h"
#include <stdbool.h>
#include <stdint.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
int bsp_encoder_init(void);
bool bsp_encoder_get_mechanical_angle_deg(float *mechanical_angle_deg);
bool bsp_encoder_get_mechanical_angle_sample(struct foc_mechanical_sample *sample);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif

#endif
