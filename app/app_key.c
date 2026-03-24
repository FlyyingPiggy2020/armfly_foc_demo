/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_key.c
 * @Author       : Codex
 * @Date         : 2026-03-20 15:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 18:20:00
 * @Brief        : 应用层按键调度实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "app_key.h"
#include "app_msgbus.h"
#include "app_protocol.h"
#include "device.h"
#include <string.h>
/*---------- macro ----------*/
#define APP_KEY_SCAN_INTERVAL_MS 10ULL
#define APP_KEY_NODE_NAME        "key"
/*---------- type define ----------*/
/* 应用层仅持有按键设备句柄和扫描节拍，不参与 GPIO 细节。 */
typedef struct {
    device_t *dev;
    uint64_t tick_last;
    struct msgbus_node node;
    msgbus_topic_t key_topic;
} app_key_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static uint32_t _app_key_get_event(void);
static void _app_key_dispatch_event(uint32_t event);
/*---------- variable ----------*/
static app_key_t g_app_key;
/*---------- function ----------*/
bool app_key_init(void)
{
    message_bus_t bus = app_msgbus_get_bus();

    memset(&g_app_key, 0, sizeof(g_app_key));

    if (bus == NULL) {
        return false;
    }

    g_app_key.dev = (device_t *)device_open("app_key");
    if (g_app_key.dev == NULL) {
        xlog_count("app_key_init: open key device failed\r\n");
        return false;
    }

    g_app_key.tick_last = get_ticks();

    if (!msgbus_node_init(&g_app_key.node, bus, APP_KEY_NODE_NAME)) {
        device_close(g_app_key.dev);
        g_app_key.dev = NULL;
        return false;
    }
    if (!msgbus_topic_register(&g_app_key.key_topic, bus, APP_PROTOCOL_KEY_TOPIC)) {
        msgbus_node_deinit(&g_app_key.node);
        memset(&g_app_key.node, 0, sizeof(g_app_key.node));
        device_close(g_app_key.dev);
        g_app_key.dev = NULL;
        return false;
    }

    /* 初始化时清空历史事件，避免系统启动后误消费旧 FIFO。 */
    device_ioctl(g_app_key.dev, IOCTL_BUTTON_CLEAR_EVENT, NULL);

    return true;
}

void app_key_process(void)
{
    uint64_t tick_now = 0U;
    uint32_t event = APP_KEY_NONE;

    if (g_app_key.dev == NULL) {
        return;
    }

    tick_now = get_ticks();

    /* 主循环若被打断，需要补齐漏掉的 10ms 扫描节拍。 */
    while ((tick_now - g_app_key.tick_last) >= APP_KEY_SCAN_INTERVAL_MS) {
        (void)device_irq_process(g_app_key.dev, 0U, NULL, 0U);
        g_app_key.tick_last += APP_KEY_SCAN_INTERVAL_MS;
    }

    while ((event = _app_key_get_event()) != APP_KEY_NONE) {
        _app_key_dispatch_event(event);
    }
}

static uint32_t _app_key_get_event(void)
{
    union button_ioctl_param param = { 0 };

    if (g_app_key.dev == NULL) {
        return APP_KEY_NONE;
    }

    if (device_ioctl(g_app_key.dev, IOCTL_BUTTON_GET_EVENT, &param) != E_OK) {
        return APP_KEY_NONE;
    }

    return param.get_event.event;
}

static void _app_key_dispatch_event(uint32_t event)
{
    struct app_protocol_key_event msg = { 0 };

    /* 按键层只负责上报输入事件，具体控制映射由 logic_key 处理。 */
    switch ((app_key_code_t)event) {
        case APP_KEY_DOWN_K1:
            msg.code = APP_KEY_DOWN_K1;
            (void)msgbus_topic_publish(g_app_key.key_topic, &msg, sizeof(msg));
            xlog_count("K1 key down\r\n");
            break;
        case APP_KEY_UP_K1:
            xlog_count("K1 key up\r\n");
            break;
        case APP_KEY_LONG_K1:
            xlog_count("K1 key long\r\n");
            break;
        case APP_KEY_DOWN_K2:
            msg.code = APP_KEY_DOWN_K2;
            (void)msgbus_topic_publish(g_app_key.key_topic, &msg, sizeof(msg));
            xlog_count("K2 key down\r\n");
            break;
        case APP_KEY_UP_K2:
            xlog_count("K2 key up\r\n");
            break;
        case APP_KEY_LONG_K2:
            xlog_count("K2 key long\r\n");
            break;
        case APP_KEY_DOWN_K3:
            msg.code = APP_KEY_DOWN_K3;
            (void)msgbus_topic_publish(g_app_key.key_topic, &msg, sizeof(msg));
            xlog_count("K3 key down\r\n");
            break;
        case APP_KEY_UP_K3:
            xlog_count("K3 key up\r\n");
            break;
        case APP_KEY_LONG_K3:
            xlog_count("K3 key long\r\n");
            break;
        case APP_KEY_NONE:
        default:
            break;
    }
}
/*---------- end of file ----------*/
