/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : main.c
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Codex
 * @LastEditTime : 2026-04-02 18:40:00
 * @Brief        : armfly-v7 板级入口
 */

/*---------- includes ----------*/
#include "app_comm.h"
#include "app_foc.h"
#include "app_key.h"
#include "app_msgbus.h"
#include "bsp_encoder.h"
#include "logic_debug.h"
#include "logic_key.h"
#include "options.h"
#include "driver.h"
#include "soft_timer.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
int main(void)
{
    cpu_config();
    _fp_timer_core_init();

    if (driver_search_device() != E_OK) {
        xlog_count("main: driver_search_device failed\r\n");
        while (1) {
        }
    }

    if (!app_msgbus_init()) {
        xlog_count("main: app_msgbus_init failed\r\n");
        while (1) {
        }
    }

    if (!app_key_init()) {
        xlog_count("main: app_key_init failed\r\n");
        while (1) {
        }
    }

    if (!app_foc_init()) {
        xlog_count("main: app_foc_init failed\r\n");
        while (1) {
        }
    }

    if (!app_comm_init()) {
        xlog_count("main: app_comm_init failed\r\n");
        while (1) {
        }
    }

    if (bsp_encoder_init() != E_OK) {
        xlog_count("main: bsp_encoder_init failed, use fallback angle\r\n");
    }

    if (!logic_key_init()) {
        xlog_count("main: logic_key_init failed\r\n");
        while (1) {
        }
    }

    if (!logic_debug_init()) {
        xlog_count("main: logic_debug_init failed\r\n");
        while (1) {
        }
    }

    while (1) {
        app_key_process();
        app_foc_process();
        app_comm_process();
        fp_timer_handler();
    }
}
/*---------- end of file ----------*/
