/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.c
 * @Author       : Codex
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-01 12:45:08
 * @Brief        : FOC 应用层实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "app_foc.h"
#include "app_msgbus.h"
#include "app_protocol.h"
#include "logic_foc_calibration.h"
#include "pmsm_foc.h"
#include <string.h>
/*---------- macro ----------*/
#define APP_FOC_DEFAULT_SPEED_REF 300.0f
#define APP_FOC_SPEED_STEP        100.0f
#define APP_FOC_LOCK_ID_REF       0.0f
#define APP_FOC_LOCK_IQ_REF       0.06f
#define APP_FOC_NODE_NAME         "foc"
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief  ADC 电流环中断入口
 * @param  irq: 中断号
 * @param  args: 中断参数
 * @param  length: 参数长度
 * @return E_OK=成功, E_ERROR=失败
 */
static int32_t _app_foc_current_loop_irq(uint32_t irq, void *args, uint32_t length);

/**
 * @brief  处理按键和调试侧发来的 FOC 服务请求
 * @param  node: 当前 service 所属节点
 * @param  req: 请求体
 * @param  req_size: 请求长度
 * @param  resp: 响应体
 * @param  resp_size: 响应长度
 * @param  user_data: 用户上下文
 * @return MSGBUS_ERR_NONE=成功, 其他=失败
 */
static int32_t _app_foc_service_handler(
    msgbus_node_t node, const void *req, uint32_t req_size, void *resp, uint32_t *resp_size, void *user_data);
/*---------- variable ----------*/
/* 应用层统一持有 FOC 实例和运行态。 */
app_foc_t g_app_foc __attribute__((section(".dtcm_data")));
/*---------- function ----------*/
/**
 * @brief  ADC 电流环中断入口
 * @param  irq: 中断号
 * @param  args: 中断参数
 * @param  length: 参数长度
 * @return E_OK=成功, E_ERROR=失败
 */
static int32_t _app_foc_current_loop_irq(uint32_t irq, void *args, uint32_t length)
{
    (void)irq;
    (void)args;
    (void)length;

    /* 标定阶段暂停零飘更新，避免注入电流污染零偏。 */
    if (logic_foc_calibration_is_active()) {
        return pmsm_foc_update_current_feedback(&g_app_foc.foc) ? E_OK : E_ERROR;
    }

    /* 停机阶段持续更新零飘，同时保持调试观测量持续刷新。 */
    if (!g_app_foc.is_started) {
        (void)foc_port_update_current_zero_drift_sample();
        return pmsm_foc_update_current_feedback(&g_app_foc.foc) ? E_OK : E_ERROR;
    }

    /* ADC DMA 回调只负责触发一次电流环计算。 */
    return pmsm_foc_current_loop(&g_app_foc.foc) ? E_OK : E_ERROR;
}

/**
 * @brief  处理按键和调试侧发来的 FOC 服务请求
 * @param  node: 当前 service 所属节点
 * @param  req: 请求体
 * @param  req_size: 请求长度
 * @param  resp: 响应体
 * @param  resp_size: 响应长度
 * @param  user_data: 用户上下文
 * @return MSGBUS_ERR_NONE=成功, 其他=失败
 */
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
                case APP_PROTOCOL_FOC_CONTROL_CMD_MOTOR_SWITCH:
                    app_foc_motor_switch();
                    break;
                case APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_INC:
                    app_foc_speed_inc();
                    break;
                case APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_DEC:
                    app_foc_speed_dec();
                    break;
                case APP_PROTOCOL_FOC_CONTROL_CMD_CALIBRATE_ELECTRICAL_ZERO:
                    logic_foc_calibration_request_start();
                    break;
                default:
                    return -MSGBUS_ERR_INVALID_ARGS;
            }
            break;
        case APP_PROTOCOL_FOC_SERVICE_CMD_GET_REALTIME:
            if ((service_resp == NULL) || (resp_size == NULL) || (*resp_size < sizeof(*service_resp))) {
                return -MSGBUS_ERR_INVALID_ARGS;
            }

            service_resp->data.foc_realtime.current_a_real = g_app_foc.foc.runtime.current_sample.a_real;
            service_resp->data.foc_realtime.current_b_real = g_app_foc.foc.runtime.current_sample.b_real;
            service_resp->data.foc_realtime.current_alpha_pu = g_app_foc.foc.runtime.current_meas_ab.alpha;
            service_resp->data.foc_realtime.current_beta_pu = g_app_foc.foc.runtime.current_meas_ab.beta;
            service_resp->data.foc_realtime.current_d_pu = g_app_foc.foc.runtime.current_meas_dq.d;
            service_resp->data.foc_realtime.current_q_pu = g_app_foc.foc.runtime.current_meas_dq.q;
            service_resp->data.foc_realtime.duty_a = g_app_foc.foc.runtime.pwm_duty.duty_a;
            service_resp->data.foc_realtime.duty_b = g_app_foc.foc.runtime.pwm_duty.duty_b;
            *resp_size = sizeof(*service_resp);
            break;
        default:
            return -MSGBUS_ERR_INVALID_ARGS;
    }

    return MSGBUS_ERR_NONE;
}

/**
 * @brief  初始化 FOC 应用层
 * @return true=成功, false=失败
 */
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

    g_app_foc.speed_ref = APP_FOC_DEFAULT_SPEED_REF;
    g_app_foc.is_started = false;
    pmsm_foc_clear_electrical_zero_offset(&g_app_foc.foc);

    return true;
}

/**
 * @brief  切换电机启停状态
 * @note   标定进行中会忽略该命令，避免锁轴过程被外部命令打断
 */
void app_foc_motor_switch(void)
{
    if (logic_foc_calibration_is_active()) {
        xlog_count("app_foc: ignore motor switch during calibration\r\n");
        return;
    }

    if (!g_app_foc.is_started) {
        pmsm_foc_start(&g_app_foc.foc, FOC_MODE_CURRENT);
        if (g_app_foc.foc.runtime.mode != FOC_MODE_CURRENT) {
            xlog_count("app_foc: motor start failed\r\n");
            return;
        }

        g_app_foc.is_started = true;
        pmsm_foc_set_current_ref(&g_app_foc.foc, APP_FOC_LOCK_ID_REF, APP_FOC_LOCK_IQ_REF);
        // pmsm_foc_set_speed_ref(&g_app_foc.foc, g_app_foc.speed_ref);
        ///* 当前工程还没有独立速度环调度，这里在改目标时同步跑一次速度 PI。 */
        // pmsm_foc_speed_loop(&g_app_foc.foc);
        // xlog_count("app_foc: motor start speed_ref=%.1f\r\n", (double)g_app_foc.speed_ref);
        return;
    }

    pmsm_foc_stop(&g_app_foc.foc);
    g_app_foc.is_started = false;
    xlog_count("app_foc: motor stop\r\n");
}

/**
 * @brief  增加速度给定
 * @note   标定进行中会忽略该命令
 */
void app_foc_speed_inc(void)
{
    if (logic_foc_calibration_is_active()) {
        xlog_count("app_foc: ignore speed inc during calibration\r\n");
        return;
    }

    g_app_foc.speed_ref += APP_FOC_SPEED_STEP;

    if (g_app_foc.is_started) {
        pmsm_foc_set_speed_ref(&g_app_foc.foc, g_app_foc.speed_ref);
        /* 当前工程还没有独立速度环调度，这里在改目标时同步跑一次速度 PI。 */
        pmsm_foc_speed_loop(&g_app_foc.foc);
    }
    xlog_count("app_foc: speed_ref inc to %.1f\r\n", (double)g_app_foc.speed_ref);
}

/**
 * @brief  减少速度给定
 * @note   标定进行中会忽略该命令
 */
void app_foc_speed_dec(void)
{
    if (logic_foc_calibration_is_active()) {
        xlog_count("app_foc: ignore speed dec during calibration\r\n");
        return;
    }

    g_app_foc.speed_ref -= APP_FOC_SPEED_STEP;

    if (g_app_foc.is_started) {
        pmsm_foc_set_speed_ref(&g_app_foc.foc, g_app_foc.speed_ref);
        /* 当前工程还没有独立速度环调度，这里在改目标时同步跑一次速度 PI。 */
        pmsm_foc_speed_loop(&g_app_foc.foc);
    }

    xlog_count("app_foc: speed_ref dec to %.1f\r\n", (double)g_app_foc.speed_ref);
}
/**
 * @brief  获取全局 FOC 控制器对象
 * @return pmsm_foc_t 控制器指针
 */
pmsm_foc_t *app_foc_get_foc(void)
{
    return &g_app_foc.foc;
}
/*---------- end of file ----------*/
