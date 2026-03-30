/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : logic_debug.c
 * @Author       : Codex
 * @Date         : 2026-03-24 12:05:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-24 12:05:00
 * @Brief        : 调试逻辑层实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "logic_debug.h"
#include "app_comm.h"
#include "app_msgbus.h"
#include "app_protocol.h"
#include "soft_timer.h"
#include <string.h>
/*---------- macro ----------*/
#define LOGIC_DEBUG_PERIOD_MS 1U
/*---------- type define ----------*/
struct logic_debug_ctx {
    msgbus_service_t foc_service;
    fp_timer_t *timer;
    bool is_init;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static void _logic_debug_timer_cb(fp_timer_t *timer);
/*---------- variable ----------*/
static struct logic_debug_ctx g_logic_debug;
/*---------- function ----------*/
static void _logic_debug_timer_cb(fp_timer_t *timer)
{
    struct app_protocol_foc_service_req req = { 0 };
    struct app_protocol_foc_service_resp resp = { 0 };
    uint32_t resp_size = sizeof(resp);

    (void)timer;

    if ((!g_logic_debug.is_init) || (g_logic_debug.foc_service == NULL)) {
        return;
    }

    req.cmd = APP_PROTOCOL_FOC_SERVICE_CMD_GET_PWM_DUTY;
    if (msgbus_service_call(g_logic_debug.foc_service, &req, sizeof(req), &resp, &resp_size) != MSGBUS_ERR_NONE) {
        return;
    }

    (void)app_comm_send_foc_pwm_duty(&resp.data.pwm_duty);
}

bool logic_debug_init(void)
{
    message_bus_t bus = app_msgbus_get_bus();

    if (bus == NULL) {
        return false;
    }

    if (g_logic_debug.is_init) {
        return true;
    }

    memset(&g_logic_debug, 0, sizeof(g_logic_debug));
    g_logic_debug.foc_service = msgbus_service_find(bus, APP_PROTOCOL_FOC_SERVICE);
    if (g_logic_debug.foc_service == NULL) {
        return false;
    }

    g_logic_debug.timer = fp_timer_create(_logic_debug_timer_cb, LOGIC_DEBUG_PERIOD_MS, NULL);
    if (g_logic_debug.timer == NULL) {
        return false;
    }

    g_logic_debug.is_init = true;
    return true;
}
/*---------- end of file ----------*/
