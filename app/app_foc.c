/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.c
 * @Author       : Codex
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 18:45:00
 * @Brief        : FOC 应用层实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "app_foc.h"
#include "app_msgbus.h"
#include "app_protocol.h"
#include <string.h>
/*---------- macro ----------*/
#define APP_FOC_DEFAULT_SPEED_REF 300.0f
#define APP_FOC_SPEED_STEP        100.0f
#define APP_FOC_NODE_NAME         "foc"
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _app_foc_current_loop_irq(uint32_t irq, void *args, uint32_t length);
static int32_t _app_foc_service_handler(
    msgbus_node_t node, const void *req, uint32_t req_size, void *resp, uint32_t *resp_size, void *user_data);
static void _app_foc_apply_speed_ref(void);
/*---------- variable ----------*/
/* 应用层统一持有 FOC 实例和运行态。 */
static app_foc_t g_app_foc __attribute__((section(".dtcm_data")));
/*---------- function ----------*/
static int32_t _app_foc_current_loop_irq(uint32_t irq, void *args, uint32_t length)
{
    (void)irq;
    (void)args;
    (void)length;

    /* ADC DMA 回调只负责触发一次电流环计算。 */
    return pmsm_foc_current_loop(&g_app_foc.foc) ? E_OK : E_ERROR;
}

static int32_t _app_foc_service_handler(
    msgbus_node_t node, const void *req, uint32_t req_size, void *resp, uint32_t *resp_size, void *user_data)
{
    const struct app_protocol_foc_service_req *info = (const struct app_protocol_foc_service_req *)req;
    struct app_protocol_foc_service_resp *service_resp = (struct app_protocol_foc_service_resp *)resp;

    (void)node;
    (void)user_data;

    if ((info == NULL) || (req_size != sizeof(*info))) {
        return -MSGBUS_ERR_INVALID_ARGS;
    }

    switch (info->cmd) {
        case APP_PROTOCOL_FOC_SERVICE_CMD_CONTROL:
            switch (info->data.control.cmd) {
                case APP_PROTOCOL_FOC_CONTROL_CMD_TOGGLE:
                    app_foc_toggle_enable();
                    break;
                case APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_UP:
                    app_foc_speed_up();
                    break;
                case APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_DOWN:
                    app_foc_speed_down();
                    break;
                default:
                    return -MSGBUS_ERR_INVALID_ARGS;
            }
            break;
        case APP_PROTOCOL_FOC_SERVICE_CMD_GET_PWM_DUTY:
            if ((service_resp == NULL) || (resp_size == NULL) || (*resp_size < sizeof(*service_resp))) {
                return -MSGBUS_ERR_INVALID_ARGS;
            }

            service_resp->data.pwm_duty.duty_a = g_app_foc.foc.runtime.pwm_duty.duty_a;
            service_resp->data.pwm_duty.duty_b = g_app_foc.foc.runtime.pwm_duty.duty_b;
            service_resp->data.pwm_duty.duty_c = g_app_foc.foc.runtime.pwm_duty.duty_c;
            *resp_size = sizeof(*service_resp);
            break;
        default:
            return -MSGBUS_ERR_INVALID_ARGS;
    }

    return MSGBUS_ERR_NONE;
}

static void _app_foc_apply_speed_ref(void)
{
    pmsm_foc_set_speed_ref(&g_app_foc.foc, g_app_foc.speed_ref);

    /* 当前工程还没有独立速度环调度，这里在改目标时同步跑一次速度 PI。 */
    pmsm_foc_speed_loop(&g_app_foc.foc);
}

bool app_foc_init(void)
{
    message_bus_t bus = app_msgbus_get_bus();

    memset(&g_app_foc, 0, sizeof(g_app_foc));

    if (bus == NULL) {
        return false;
    }

    /* 板级层负责 profile 和 HAL 适配，应用层只做控制器装配。 */
    if (!foc_port_init()) {
        return false;
    }

    if (!msgbus_node_init(&g_app_foc.node, bus, APP_FOC_NODE_NAME)) {
        return false;
    }

    if (!pmsm_foc_init(&g_app_foc.foc,
                       foc_port_get_motor_profile(),
                       foc_port_get_power_stage_profile(),
                       foc_port_get_sensor_profile(),
                       foc_port_get_ctrl_cfg(),
                       foc_port_get_hal_ops(),
                       foc_port_get_hal_user_data())) {
        xlog_count("app_foc_init: pmsm_foc_init failed\r\n");
        msgbus_node_deinit(&g_app_foc.node);
        memset(&g_app_foc.node, 0, sizeof(g_app_foc.node));
        return false;
    }

    if (!msgbus_service_advertise(
            &g_app_foc.foc_service, &g_app_foc.node, APP_PROTOCOL_FOC_SERVICE, _app_foc_service_handler, NULL)) {
        memset(&g_app_foc.foc, 0, sizeof(g_app_foc.foc));
        msgbus_node_deinit(&g_app_foc.node);
        memset(&g_app_foc.node, 0, sizeof(g_app_foc.node));
        return false;
    }

    /* 电流环由 motor_adc 的 DMA 完成中断驱动。 */
    if (!foc_port_register_current_loop_irq(_app_foc_current_loop_irq)) {
        xlog_count("app_foc_init: register current loop irq failed\r\n");
        memset(&g_app_foc.foc, 0, sizeof(g_app_foc.foc));
        msgbus_node_deinit(&g_app_foc.node);
        memset(&g_app_foc.node, 0, sizeof(g_app_foc.node));
        g_app_foc.foc_service = NULL;
        return false;
    }
    pmsm_foc_start(&g_app_foc.foc, FOC_MODE_SPEED);
    g_app_foc.speed_ref = 0;
    g_app_foc.is_started = false;
    

    return true;
}

void app_foc_toggle_enable(void)
{
//    if (!g_app_foc.is_started) {
//        pmsm_foc_start(&g_app_foc.foc, FOC_MODE_SPEED);
//        g_app_foc.is_started = true;
//        _app_foc_apply_speed_ref();
//        xlog_count("app_foc: start speed_ref=%.1f\r\n", (double)g_app_foc.speed_ref);
//        return;
//    }

//    pmsm_foc_stop(&g_app_foc.foc);
//    g_app_foc.is_started = false;
//    xlog_count("app_foc: stop\r\n");
}

void app_foc_speed_up(void)
{
    g_app_foc.speed_ref += APP_FOC_SPEED_STEP;

    if (g_app_foc.is_started) {
        _app_foc_apply_speed_ref();
    }
    xlog_count("app_foc: speed_ref=%.1f\r\n", (double)g_app_foc.speed_ref);
}

void app_foc_speed_down(void)
{
    g_app_foc.speed_ref -= APP_FOC_SPEED_STEP;

    if (g_app_foc.is_started) {
        _app_foc_apply_speed_ref();
    }

    xlog_count("app_foc: speed_ref=%.1f\r\n", (double)g_app_foc.speed_ref);
}

pmsm_foc_t *app_foc_get_foc(void)
{
    return &g_app_foc.foc;
}
/*---------- end of file ----------*/
