/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : bsp_encoder.c
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-02 16:20:00
 * @Brief        : armfly-v7 板级磁编码器实时获取实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "bsp_encoder.h"
#include "cpu.h"
#include "tle5012b/tle5012b_sensor.h"
/*---------- macro ----------*/
#define BSP_ENCODER_DEVICE_NAME "spi1_tle5012b"
/*---------- type define ----------*/
struct bsp_encoder_ctx {
    device_t *spi_dev;
    struct tle5012b_sensor sensor;
    bool is_ready;
};
/*---------- variable prototype ----------*/
static float _bsp_encoder_wrap_angle_deg(float angle_deg);
static bool _bsp_encoder_read_angle_data(struct tle5012b_angle_data *angle_data);
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static struct bsp_encoder_ctx g_bsp_encoder_ctx = { 0 };
/*---------- function ----------*/
static float _bsp_encoder_wrap_angle_deg(float angle_deg)
{
    while (angle_deg >= 360.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg < 0.0f) {
        angle_deg += 360.0f;
    }

    return angle_deg;
}

static bool _bsp_encoder_read_angle_data(struct tle5012b_angle_data *angle_data)
{
    if (angle_data == NULL) {
        return false;
    }
    if (!g_bsp_encoder_ctx.is_ready) {
        return false;
    }

    return tle5012b_sensor_read_angle(&g_bsp_encoder_ctx.sensor, angle_data) == E_OK;
}

static bool _bsp_encoder_read_speed_data(struct tle5012b_speed_data *speed_data)
{
    if (speed_data == NULL) {
        return false;
    }
    if (!g_bsp_encoder_ctx.is_ready) {
        return false;
    }

    return tle5012b_sensor_read_speed(&g_bsp_encoder_ctx.sensor, speed_data) == E_OK;
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

bool bsp_encoder_get_mechanical_angle_deg(float *mechanical_angle_deg)
{
    struct tle5012b_angle_data angle_data = { 0 };

    if (mechanical_angle_deg == NULL) {
        return false;
    }
    if (!_bsp_encoder_read_angle_data(&angle_data)) {
        return false;
    }

    *mechanical_angle_deg = _bsp_encoder_wrap_angle_deg(angle_data.mechanical_angle_deg);

    return true;
}

bool bsp_encoder_get_mechanical_angle_sample(struct foc_mechanical_sample *sample)
{
    struct tle5012b_angle_data angle_data = { 0 };
    struct tle5012b_speed_data speed_data = { 0 };

    if (sample == NULL) {
        return false;
    }

    *sample = (struct foc_mechanical_sample){ 0 };

    if (!_bsp_encoder_read_angle_data(&angle_data)) {
        return false;
    }
    if (!_bsp_encoder_read_speed_data(&speed_data)) {
        return false;
    }

    sample->angle_deg = _bsp_encoder_wrap_angle_deg(angle_data.mechanical_angle_deg);
    sample->speed_deg_s = speed_data.mechanical_speed_deg_s;

    return true;
}
/*---------- end of file ----------*/
