/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_msgbus.c
 * @Author       : Codex
 * @Date         : 2026-03-20 20:30:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 20:30:00
 * @Brief        : 应用层消息总线基础设施
 */

/*---------- includes ----------*/
#include "options.h"
#include "app_msgbus.h"
#include <string.h>
/*---------- macro ----------*/
#define APP_MSGBUS_NAME "app_bus"
/*---------- type define ----------*/
/* 总线层只负责 bus 本体生命周期，不再直接装配业务节点。 */
struct app_msgbus_ctx {
    struct message_bus bus;
    bool is_init;
};
/*---------- variable ----------*/
static struct app_msgbus_ctx g_app_msgbus;
/*---------- function ----------*/
bool app_msgbus_init(void)
{
    bool retval = true;

    if (g_app_msgbus.is_init) {
        return true;
    }

    memset(&g_app_msgbus, 0, sizeof(g_app_msgbus));
    msgbus_init(&g_app_msgbus.bus, APP_MSGBUS_NAME);

    if (!retval) {
        msgbus_deinit(&g_app_msgbus.bus);
        memset(&g_app_msgbus, 0, sizeof(g_app_msgbus));
        xlog_error("app_msgbus_init failed\r\n");
        return false;
    }

    g_app_msgbus.is_init = true;
    return true;
}

message_bus_t app_msgbus_get_bus(void)
{
    if (!g_app_msgbus.is_init) {
        return NULL;
    }

    return &g_app_msgbus.bus;
}
/*---------- end of file ----------*/
