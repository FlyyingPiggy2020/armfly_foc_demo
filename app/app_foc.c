/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.c
 * @Author       : Codex
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17 16:48:00
 * @Brief        : FOC 应用封装入口实现
 */

/*---------- includes ----------*/
#include "app_foc.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
bool app_foc_init(app_foc_t *app)
{
    if (app == NULL) {
        return false;
    }

    memset(app, 0, sizeof(*app));

    /* 应用层当前只负责启动板级 FOC 端口，具体参数与设备绑定由 foc_port 内部组织 */
    return foc_port_init(&app->port);
}

pmsm_foc_t *app_foc_get_foc(app_foc_t *app)
{
    if (app == NULL) {
        return NULL;
    }

    return foc_port_get_foc(&app->port);
}
/*---------- end of file ----------*/
