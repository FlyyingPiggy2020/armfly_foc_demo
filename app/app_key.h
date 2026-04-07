/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_key.h
 * @Author       : Codex
 * @Date         : 2026-03-20 15:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 16:20:00
 * @Brief        : 应用层按键调度接口
 */

#ifndef __APP_KEY_H__
#define __APP_KEY_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "button.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/* 应用层只关心逻辑按键编号，不感知底层 GPIO 映射。 */
typedef enum {
    APP_KEY_ID_K1 = 0,
    APP_KEY_ID_K2,
    APP_KEY_ID_K3,
    APP_KEY_ID_COUNT,
} app_key_id_t;

/* 事件编码直接复用通用 button 驱动的事件格式。 */
typedef enum {
    APP_KEY_NONE = BUTTON_EVENT_NONE_CODE,
    APP_KEY_DOWN_K1 = BUTTON_EVENT_CODE(APP_KEY_ID_K1, BUTTON_EVENT_DOWN),
    APP_KEY_SHORT_UP_K1 = BUTTON_EVENT_CODE(APP_KEY_ID_K1, BUTTON_EVENT_SHORT_UP),
    APP_KEY_LONG_K1 = BUTTON_EVENT_CODE(APP_KEY_ID_K1, BUTTON_EVENT_LONG),
    APP_KEY_LONG_UP_K1 = BUTTON_EVENT_CODE(APP_KEY_ID_K1, BUTTON_EVENT_LONG_UP),
    APP_KEY_DOWN_K2 = BUTTON_EVENT_CODE(APP_KEY_ID_K2, BUTTON_EVENT_DOWN),
    APP_KEY_SHORT_UP_K2 = BUTTON_EVENT_CODE(APP_KEY_ID_K2, BUTTON_EVENT_SHORT_UP),
    APP_KEY_LONG_K2 = BUTTON_EVENT_CODE(APP_KEY_ID_K2, BUTTON_EVENT_LONG),
    APP_KEY_LONG_UP_K2 = BUTTON_EVENT_CODE(APP_KEY_ID_K2, BUTTON_EVENT_LONG_UP),
    APP_KEY_DOWN_K3 = BUTTON_EVENT_CODE(APP_KEY_ID_K3, BUTTON_EVENT_DOWN),
    APP_KEY_SHORT_UP_K3 = BUTTON_EVENT_CODE(APP_KEY_ID_K3, BUTTON_EVENT_SHORT_UP),
    APP_KEY_LONG_K3 = BUTTON_EVENT_CODE(APP_KEY_ID_K3, BUTTON_EVENT_LONG),
    APP_KEY_LONG_UP_K3 = BUTTON_EVENT_CODE(APP_KEY_ID_K3, BUTTON_EVENT_LONG_UP),
} app_key_code_t;
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
bool app_key_init(void);
void app_key_process(void);
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif /* __APP_KEY_H__ */
