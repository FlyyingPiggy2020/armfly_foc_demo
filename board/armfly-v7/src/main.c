/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : main.c
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17 16:48:00
 * @Brief        : armfly-v7 board entry point
 */

/*---------- includes ----------*/
#include "app_foc.h"
#include "options.h"
#include "driver.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
int main(void)
{
    cpu_config();

    if (driver_search_device() != E_OK) {
        xlog_count("main: driver_search_device failed\r\n");
        while (1) {
        }
    }

    if (!app_foc_init())
        {
            xlog_count("main: app_foc_init failed\r\n");
            while (1) {
            }
        }

    while (1) {
    }
}
/*---------- end of file ----------*/
