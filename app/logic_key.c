/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : logic_key.c
 * @Author       : Codex
 * @Date         : 2026-03-21 09:30:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-24 11:05:00
 * @Brief        : 按键逻辑映射层
 */

/*---------- includes ----------*/
#include "logic_key.h"
#include "app_msgbus.h"
#include "app_protocol.h"
#include <string.h>
/*---------- macro ----------*/
#define LOGIC_KEY_NODE_NAME "logic_key"
/*---------- type define ----------*/
/* 按键逻辑层负责把输入事件翻译成统一的 FOC 控制命令。 */
struct logic_key_ctx {
    struct msgbus_node node;
    msgbus_topic_t key_topic;
    msgbus_service_t foc_service;
    bool is_init;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static void _logic_key_on_key_event(
    msgbus_node_t node, const char *topic, const void *msg, uint32_t size, void *user_data);
/*---------- variable ----------*/
static struct logic_key_ctx g_logic_key;
/*---------- function ----------*/
static void _logic_key_on_key_event(
    msgbus_node_t node, const char *topic, const void *msg, uint32_t size, void *user_data)
{
    const struct app_protocol_key_event *event = (const struct app_protocol_key_event *)msg;
    struct app_protocol_foc_service_req req = { 0 };

    (void)node;
    (void)topic;
    (void)user_data;

    if ((event == NULL) || (size != sizeof(*event)) || (g_logic_key.foc_service == NULL)) {
        return;
    }

    req.cmd = APP_PROTOCOL_FOC_SERVICE_CMD_CONTROL;

    switch (event->code) {
        case APP_KEY_DOWN_K1:
            req.data.control.cmd = APP_PROTOCOL_FOC_CONTROL_CMD_TOGGLE;
            break;
        case APP_KEY_DOWN_K2:
            req.data.control.cmd = APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_UP;
            break;
        case APP_KEY_DOWN_K3:
            req.data.control.cmd = APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_DOWN;
            break;
        default:
            return;
    }

    (void)msgbus_service_call(g_logic_key.foc_service, &req, sizeof(req), NULL, NULL);
}

bool logic_key_init(void)
{
    message_bus_t bus = app_msgbus_get_bus();

    if (bus == NULL) {
        return false;
    }

    if (g_logic_key.is_init) {
        return true;
    }

    memset(&g_logic_key, 0, sizeof(g_logic_key));
    if (!msgbus_node_init(&g_logic_key.node, bus, LOGIC_KEY_NODE_NAME)) {
        return false;
    }

    g_logic_key.key_topic = msgbus_topic_find(bus, APP_PROTOCOL_KEY_TOPIC);
    g_logic_key.foc_service = msgbus_service_find(bus, APP_PROTOCOL_FOC_SERVICE);
    if ((g_logic_key.key_topic == NULL) || (g_logic_key.foc_service == NULL)) {
        msgbus_node_deinit(&g_logic_key.node);
        memset(&g_logic_key, 0, sizeof(g_logic_key));
        return false;
    }

    if (!msgbus_topic_subscribe(&g_logic_key.node, g_logic_key.key_topic, _logic_key_on_key_event, NULL)) {
        msgbus_node_deinit(&g_logic_key.node);
        memset(&g_logic_key, 0, sizeof(g_logic_key));
        return false;
    }

    g_logic_key.is_init = true;
    return true;
}
/*---------- end of file ----------*/
