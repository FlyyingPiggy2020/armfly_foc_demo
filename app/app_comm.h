/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_comm.h
 * @Author       : Codex
 * @Date         : 2026-03-24 10:55:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-24 12:05:00
 * @Brief        : 应用层串口调试传输接口
 */

#ifndef __APP_COMM_H__
#define __APP_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdbool.h>
#include "app_protocol.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
bool app_comm_init(void);
bool app_comm_send_foc_pwm_duty(const struct app_protocol_foc_pwm_duty *duty);
void app_comm_process(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __APP_COMM_H__ */
