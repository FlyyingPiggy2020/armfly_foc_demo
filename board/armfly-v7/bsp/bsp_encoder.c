/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : bsp_encoder.c
 * @Author       : Codex
 * @Date         : 2026-03-25 10:30:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-25 10:30:00
 * @Brief        : armfly-v7 板级磁编码器实时获取实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "bsp_encoder.h"
#include "bsp_foc.h"
#include "cpu.h"
#include "tle5012b/tle5012b_sensor.h"
/*---------- macro ----------*/
#define BSP_ENCODER_DEVICE_NAME  "spi1_tle5012b"
/*---------- type define ----------*/
struct bsp_encoder_ctx {
    device_t *spi_dev;
    struct tle5012b_sensor sensor;
    bool is_ready;
};
/*---------- variable prototype ----------*/
static foc_angle_t _bsp_encoder_wrap_angle_deg(foc_angle_t angle);
static foc_angle_t _bsp_encoder_convert_mechanical_to_electrical(foc_angle_t mechanical_angle_deg);
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static struct bsp_encoder_ctx g_bsp_encoder_ctx = { 0 };
/*---------- function ----------*/
static foc_angle_t _bsp_encoder_wrap_angle_deg(foc_angle_t angle)
{
    while (angle >= 360.0f) {
        angle -= 360.0f;
    }
    while (angle < 0.0f) {
        angle += 360.0f;
    }

    return angle;
}

static foc_angle_t _bsp_encoder_convert_mechanical_to_electrical(foc_angle_t mechanical_angle_deg)
{
    const motor_profile_t *motor_profile = foc_port_get_motor_profile();
    const sensor_profile_t *sensor_profile = foc_port_get_sensor_profile();
    foc_scalar_t direction = 1.0f;
    foc_angle_t electrical_angle = 0.0f;

    if ((motor_profile == NULL) || (sensor_profile == NULL)) {
        return 0.0f;
    }

    direction = (foc_scalar_t)sensor_profile->encoder_direction * (foc_scalar_t)motor_profile->electrical_direction;
    electrical_angle = mechanical_angle_deg * direction * (foc_scalar_t)motor_profile->pole_pairs;
    electrical_angle += motor_profile->electrical_offset;
    electrical_angle += sensor_profile->electrical_offset;

    return _bsp_encoder_wrap_angle_deg(electrical_angle);
}

int bsp_encoder_init(void)
{
    if (g_bsp_encoder_ctx.is_ready) {
        return E_OK;
    }

    g_bsp_encoder_ctx.spi_dev = (device_t *)device_open(BSP_ENCODER_DEVICE_NAME);
    if (g_bsp_encoder_ctx.spi_dev == NULL) {
        xlog_count("bsp_encoder_init: open %s failed\r\n", BSP_ENCODER_DEVICE_NAME);
        return E_ERROR;
    }

    if (tle5012b_sensor_init(&g_bsp_encoder_ctx.sensor, g_bsp_encoder_ctx.spi_dev) != E_OK) {
        xlog_count("bsp_encoder_init: tle5012b_sensor_init failed\r\n");
        device_close(g_bsp_encoder_ctx.spi_dev);
        g_bsp_encoder_ctx.spi_dev = NULL;
        return E_ERROR;
    }

    g_bsp_encoder_ctx.is_ready = true;

    return E_OK;
}

bool bsp_encoder_get_angle_sample(uint32_t target_tick_us, foc_angle_sample_t *sample)
{
    struct tle5012b_angle_data angle_data = { 0 };
    uint32_t angle_tick_us = 0U;

    if (sample == NULL) {
        return false;
    }

    *sample = (foc_angle_sample_t){ 0 };

    if (!g_bsp_encoder_ctx.is_ready) {
        return false;
    }
    if (tle5012b_sensor_read_angle(&g_bsp_encoder_ctx.sensor, &angle_data) != E_OK) {
        return false;
    }

    if (target_tick_us != 0U) {
        angle_tick_us = target_tick_us;
    } else {
        angle_tick_us = (uint32_t)(tick_get() * 1000ULL);
    }

    sample->electrical_angle = _bsp_encoder_convert_mechanical_to_electrical(angle_data.mechanical_angle_deg);
    sample->electrical_speed = 0.0f;
    sample->angle_tick_us = angle_tick_us;
    sample->status = FOC_ANGLE_STATUS_VALID;

    return true;
}
/*---------- end of file ----------*/
