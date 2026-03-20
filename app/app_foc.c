/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_foc.c
 * @Author       : Codex
 * @Date         : 2026-03-17 15:55:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 09:18:29
 * @Brief        : FOC 应用封装入口实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "app_foc.h"
#include "analog.h"
#include <string.h>
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static int32_t _app_foc_current_loop_irq(uint32_t irq, void *args, uint32_t length);
static void _app_foc_close_devices(void);
/*---------- variable ----------*/
static app_foc_t g_app_foc __attribute__((section(".dtcm_data")));
/*---------- function ----------*/
static int32_t _app_foc_current_loop_irq(uint32_t irq, void *args, uint32_t length)
{
    (void)irq;
    (void)args;
    (void)length;

    return pmsm_foc_current_loop(&g_app_foc.foc) ? E_OK : E_ERROR;
}

static void _app_foc_close_devices(void)
{
    if (g_app_foc.current_dev != NULL) {
        device_close(g_app_foc.current_dev);
        g_app_foc.current_dev = NULL;
    }

    if (g_app_foc.pwm_dev != NULL) {
        device_close(g_app_foc.pwm_dev);
        g_app_foc.pwm_dev = NULL;
    }
}

bool app_foc_init(void)
{
    memset(&g_app_foc, 0, sizeof(g_app_foc));

    if (!foc_port_init()) {
        return false;
    }

    g_app_foc.pwm_dev = (device_t *)device_open("motor_pwm");
    g_app_foc.current_dev = (device_t *)device_open("motor_adc");
    if ((g_app_foc.pwm_dev == NULL) || (g_app_foc.current_dev == NULL)) {
        xlog_count("app_foc_init: open device failed adc=%p pwm=%p\r\n", g_app_foc.current_dev, g_app_foc.pwm_dev);
        _app_foc_close_devices();
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
        _app_foc_close_devices();
        return false;
    }

    if (device_ioctl(g_app_foc.current_dev, IOCTL_ANALOG_SET_IRQ_HANDLER, _app_foc_current_loop_irq) != E_OK) {
        xlog_count("app_foc_init: register current loop irq failed\r\n");
        _app_foc_close_devices();
        memset(&g_app_foc.foc, 0, sizeof(g_app_foc.foc));
        return false;
    }

    return true;
}

pmsm_foc_t *app_foc_get_foc(void)
{
    return &g_app_foc.foc;
}
/*---------- end of file ----------*/
