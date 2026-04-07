/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_comm.c
 * @Author       : Codex
 * @Date         : 2026-03-24 10:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-06 15:55:39
 * @Brief        : 应用层串口调试传输实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "app_comm.h"
#include "app_foc.h"
#include "device.h"
#include "vofa/vofa.h"
#include <string.h>
/*---------- macro ----------*/
#define APP_COMM_DEV_NAME "usart1"
/*---------- type define ----------*/
struct app_comm_ctx {
    device_t *uart_dev;
    bool is_init;
};
/*---------- variable prototype ----------*/
/*---------- variable ----------*/
static struct app_comm_ctx g_app_comm;
/*---------- function ----------*/
bool app_comm_init(void)
{
    if (g_app_comm.is_init) {
        return true;
    }

    memset(&g_app_comm, 0, sizeof(g_app_comm));

    g_app_comm.uart_dev = (device_t *)device_open(APP_COMM_DEV_NAME);
    if (g_app_comm.uart_dev == NULL) {
        xlog_count("app_comm_init: open %s failed\r\n", APP_COMM_DEV_NAME);
        return false;
    }

    if (vofa_init(g_app_comm.uart_dev) != 0) {
        xlog_count("app_comm_init: vofa_init failed\r\n");
        return false;
    }

    g_app_comm.is_init = true;

    return true;
}
float debug_data[8] = { 0 };
bool app_comm_send_foc_realtime(const struct app_protocol_foc_realtime *realtime)
{
    const struct foc_debug_sample *debug_sample = NULL;
    float values[8] = { 0 };

    (void)realtime;

    if ((!g_app_comm.is_init) || (g_app_comm.uart_dev == NULL)) {
        return false;
    }

    debug_sample = app_foc_get_debug_sample();
    if (debug_sample == NULL) {
        return false;
    }

    values[0] = debug_sample->ia;
    values[1] = debug_sample->ib;
    values[2] = debug_sample->mech_speed_rad_s;
    values[3] = debug_sample->extra[0];
    values[4] = debug_sample->electrical_angle_deg;
    values[5] = debug_sample->current_d_pu;
    values[6] = debug_sample->current_q_pu;
    values[7] = debug_data[0];
    return (vofa_send(values, 8U) == 0);
}

void app_comm_process(void)
{
    /* 当前调试发送由 logic_debug 的 1ms 软定时驱动，这里预留给后续通信任务扩展。 */
}
/*---------- end of file ----------*/
