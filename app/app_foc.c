/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.c
 * @Author       : lxf
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-06 10:47:04
 * @Brief        : FOC 应用层实现
 */

/*---------- includes ----------*/
#include "foc.h"
#include "foc_types.h"
#include "options.h"
#include "app_foc.h"
#include "app_msgbus.h"
#include "app_protocol.h"
#include "bsp_foc.h"
#include "message_bus.h"
#include <string.h>
/*---------- macro ----------*/
#define APP_FOC_NODE_NAME "foc"
/*---------- type define ----------*/
struct app_foc_ctx {
    struct foc_motor foc;
    struct msgbus_node node;
    msgbus_service_t foc_service;
    foc_scalar_t current_ref;
    foc_scalar_t speed_ref;
};
/*---------- variable prototype ----------*/
static bool _app_foc_is_running_mode(enum foc_mode mode);
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
struct app_foc_ctx g_app_foc __attribute__((section(".dtcm_data")));
/*---------- function ----------*/
static bool _app_foc_is_running_mode(enum foc_mode mode)
{
    return (mode == FOC_MODE_CURRENT) || (mode == FOC_MODE_SPEED);
}

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

    if (!foc_run_fast(&g_app_foc.foc)) {
        return E_ERROR;
    }

    return E_OK;
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
                    foc_command_align(&g_app_foc.foc);
                    break;
                default:
                    return -MSGBUS_ERR_INVALID_ARGS;
            }
            break;
        case APP_PROTOCOL_FOC_SERVICE_CMD_GET_REALTIME:
            if ((service_resp == NULL) || (resp_size == NULL) || (*resp_size < sizeof(*service_resp))) {
                return -MSGBUS_ERR_INVALID_ARGS;
            }
            memset(&service_resp->data.foc_realtime, 0, sizeof(service_resp->data.foc_realtime));
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

    if (!foc_init(&g_app_foc.foc, foc_port_get_config(), foc_port_get_port(), foc_port_get_port_ctx())) {
        xlog_count("app_foc_init: foc_init failed\r\n");
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

    g_app_foc.speed_ref = 0;
    g_app_foc.current_ref = 0;
    return true;
}

const struct foc_debug_sample *app_foc_get_debug_sample(void)
{
    return &g_app_foc.foc.debug_sample;
}

/**
 * @brief  运行 FOC 慢环任务
 */
void app_foc_process(void)
{
    foc_run_slow(&g_app_foc.foc, (uint32_t)get_ticks());
}

/**
 * @brief  切换电机启停状态
 * @note   对齐进行中会忽略该命令，避免零位标定被外部命令打断
 */
void app_foc_motor_switch(void)
{
    enum foc_mode mode = g_app_foc.foc.state.mode;

    if (mode == FOC_MODE_ALIGN) {
        xlog_count("app_foc: ignore motor switch during align\r\n");
        return;
    }

    if (!_app_foc_is_running_mode(mode)) {
        foc_command_current(&g_app_foc.foc, 0, g_app_foc.current_ref);
        if (!_app_foc_is_running_mode(g_app_foc.foc.state.mode)) {
            xlog_count("app_foc: motor start failed\r\n");
        }
        return;
    }

    foc_command_disable(&g_app_foc.foc);
    xlog_count("app_foc: motor disable\r\n");
}

/**
 * @brief
 * @note   对齐进行中会忽略该命令
 */
void app_foc_speed_inc(void)
{
    enum foc_mode mode = g_app_foc.foc.state.mode;

    if (mode == FOC_MODE_ALIGN) {
        xlog_count("app_foc: ignore speed inc during align\r\n");
        return;
    }

    if (mode == FOC_MODE_CURRENT) {
        g_app_foc.current_ref += 0.005f;
        if (g_app_foc.current_ref >= 0.95f) {
            g_app_foc.current_ref = 0.95f;
        }
        foc_command_current(&g_app_foc.foc, 0, g_app_foc.current_ref);
    }

    //    xlog_count("app_foc: speed_ref inc to %.1f\r\n", (double)g_app_foc.speed_ref);
}

/**
 * @brief  减少速度给定
 * @note   对齐进行中会忽略该命令
 */
void app_foc_speed_dec(void)
{
    enum foc_mode mode = g_app_foc.foc.state.mode;

    if (mode == FOC_MODE_ALIGN) {
        xlog_count("app_foc: ignore speed inc during align\r\n");
        return;
    }

    if (mode == FOC_MODE_CURRENT) {
        g_app_foc.current_ref -= 0.005f;
        if (g_app_foc.current_ref <= 0.0f) {
            g_app_foc.current_ref = 0.0f;
        }
        foc_command_current(&g_app_foc.foc, 0, g_app_foc.current_ref);
    }

    //    xlog_count("app_foc: speed_ref inc to %.1f\r\n", (double)g_app_foc.speed_ref);
}
/*---------- end of file ----------*/
