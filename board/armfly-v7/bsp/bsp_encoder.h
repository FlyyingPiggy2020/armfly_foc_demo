/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : bsp_encoder.h
 * @Author       : Codex
 * @Date         : 2026-03-25 10:30:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-25 10:30:00
 * @Brief        : armfly-v7 板级磁编码器实时读取接口
 */

#ifndef __BSP_ENCODER_H__
#define __BSP_ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "foc_types.h"
#include <stdbool.h>
#include <stdint.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  初始化板级磁编码器
 * @return E_OK=成功, 其他=失败
 */
int bsp_encoder_init(void);

/**
 * @brief  实时读取当前电角度样本
 * @param  target_tick_us: 当前目标时刻，非 0 时用于对齐输出时间戳
 * @param  sample: 输出角度样本
 * @return true=读取成功, false=读取失败
 */
bool bsp_encoder_get_angle_sample(uint32_t target_tick_us, foc_angle_sample_t *sample);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif

#endif
