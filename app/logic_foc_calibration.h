/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : logic_foc_calibration.h
 * @Author       : Codex
 * @Date         : 2026-03-30 11:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-30 11:00:00
 * @Brief        : 电角度零位标定逻辑接口
 */

#ifndef __LOGIC_FOC_CALIBRATION_H__
#define __LOGIC_FOC_CALIBRATION_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdbool.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
bool logic_foc_calibration_init(void);
void logic_foc_calibration_process(void);
void logic_foc_calibration_request_start(void);
bool logic_foc_calibration_is_active(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __LOGIC_FOC_CALIBRATION_H__ */
