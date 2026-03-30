/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_comm.c
 * @Author       : Codex
 * @Date         : 2026-03-24 10:55:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-24 12:05:00
 * @Brief        : 应用层串口调试传输实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "app_comm.h"
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

bool app_comm_send_foc_pwm_duty(const struct app_protocol_foc_pwm_duty *duty)
{
    float values[3];

    if ((!g_app_comm.is_init) || (g_app_comm.uart_dev == NULL) || (duty == NULL)) {
        return false;
    }

    values[0] = duty->duty_a;
    values[1] = duty->duty_b;
    values[2] = duty->duty_c;

    return (vofa_send(values, 3U) == 0);
}

void app_comm_process(void)
{
    /* 当前调试发送由 logic_debug 的 1ms 软定时驱动，这里预留给后续通信任务扩展。 */
}
/*---------- end of file ----------*/
