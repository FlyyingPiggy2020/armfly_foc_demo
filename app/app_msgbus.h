/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_msgbus.h
 * @Author       : Codex
 * @Date         : 2026-03-20 20:30:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 20:30:00
 * @Brief        : 应用层消息总线接口
 */

#ifndef __APP_MSGBUS_H__
#define __APP_MSGBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdbool.h>
#include "message_bus.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
bool app_msgbus_init(void);
message_bus_t app_msgbus_get_bus(void);
/*---------- end of file ----------*/

#ifdef __cplusplus
}
#endif
#endif /* __APP_MSGBUS_H__ */
