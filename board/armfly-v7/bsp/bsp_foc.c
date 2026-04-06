/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : bsp_foc.c
 * @Author       : lxf
 * @Date         : 2026-04-02 16:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-04-03 14:25:25
 * @Brief        : armfly-v7 FOC 板级聚合实现
 */

/*---------- includes ----------*/
#include "errorno.h"
#include "options.h"
#include "bsp_encoder.h"
#include "bsp_foc.h"
#include "analog.h"
#include "butter/butter.h"
#include "pwmc.h"
/*---------- macro ----------*/
#define FOC_CURRENT_ADC_MID_CODE                 32767.0f
#define FOC_CURRENT_ZERO_DRIFT_FILTER_B0         0.00391139f
#define FOC_CURRENT_ZERO_DRIFT_FILTER_B1         0.00391139f
#define FOC_CURRENT_ZERO_DRIFT_FILTER_A1         -0.99217722f
#define FOC_ADC_CODE_TO_CURRENT_SCALE_A_PER_CODE 5.0354772e-5f // 将ADC转为实际电流的系数
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static void _foc_port_reset_zero_drift_filter(FilterCoefficientsFloat *filter);
static bool _foc_port_read_fast_sample(void *user_data, struct foc_fast_sample *sample);
static bool _foc_port_read_mechanical_angle(void *user_data, struct foc_mechanical_sample *sample);
static uint32_t _foc_port_get_tick_ms(void *user_data);
static bool _foc_port_write_pwm(void *user_data, const struct foc_pwm_out *pwm);
static bool _foc_port_update_current_zero_drift(void *user_data);
/*---------- variable ----------*/
static struct foc_config g_foc_config = {
    .motor = {
        .pole_pairs = 7U,
        .angle_direction = 1,
        .current_base_a = 1.0f,
        .voltage_base_v = 6.9282f,
    },
    .ctrl = {
        .id_kp_pu = 0.5079f,
        .id_ki_pu = 0.1156f,
        .id_limit_pu = 0.5f,
        .iq_kp_pu = 0.5079f,
        .iq_ki_pu = 0.1156f,
        .iq_limit_pu = 0.5f,
        .speed_kp = 0.0f,
        .speed_ki = 0.0f,
        .speed_limit_pu = 0.0f,
        .current_loop_hz = 8000U,
        .speed_loop_hz = 200U,
    },
    .align = {
        .voltage_d_pu = 0.12f,
    },
};

static const struct foc_port g_foc_port = {
    .read_fast_sample = _foc_port_read_fast_sample,
    .read_mech_angle = _foc_port_read_mechanical_angle,
    .get_tick_ms = _foc_port_get_tick_ms,
    .write_pwm = _foc_port_write_pwm,
    .update_current_zero_drift = _foc_port_update_current_zero_drift,
};

static struct foc_port_ctx g_foc_port_ctx = { 0 };
static FilterCoefficientsFloat g_current_a_offset_filter = { 0 };
static FilterCoefficientsFloat g_current_b_offset_filter = { 0 };
static float g_current_a_offset = FOC_CURRENT_ADC_MID_CODE;
static float g_current_b_offset = FOC_CURRENT_ADC_MID_CODE;
/*---------- function ----------*/
static void _foc_port_reset_zero_drift_filter(FilterCoefficientsFloat *filter)
{
    if (filter == NULL) {
        return;
    }

    filter->b[0] = FOC_CURRENT_ZERO_DRIFT_FILTER_B0;
    filter->b[1] = FOC_CURRENT_ZERO_DRIFT_FILTER_B1;
    filter->a[0] = 1.0f;
    filter->a[1] = FOC_CURRENT_ZERO_DRIFT_FILTER_A1;
    filter->last_input = FOC_CURRENT_ADC_MID_CODE;
    filter->last_output = FOC_CURRENT_ADC_MID_CODE;
}

/**
 * @brief 读取电流、速度、角度、输出结果为实际值
 * @param {void} *user_data
 * @param {foc_fast_sample} *sample
 * @return {*}
 */
static bool _foc_port_read_fast_sample(void *user_data, struct foc_fast_sample *sample)
{
    union analog_ioctl_param param = { 0 };
    struct foc_mechanical_sample mechanical_sample = { 0 };
    struct foc_port_ctx *ctx = (struct foc_port_ctx *)user_data;
    uint32_t sample_tick_us = 0U;
    uint16_t a_adc = 0U;
    uint16_t b_adc = 0U;

    if (sample == NULL) {
        return false;
    }

    *sample = (struct foc_fast_sample){ 0 };

    if (ctx == NULL) {
        ctx = &g_foc_port_ctx;
    }
    if (ctx->current_dev == NULL) {
        return false;
    }

    param.get.channel = 1U;
    if (device_ioctl(ctx->current_dev, IOCTL_ANALOG_GET, &param) != E_OK) {
        return false;
    }
    a_adc = (uint16_t)param.get.data;

    param.get.channel = 2U;
    if (device_ioctl(ctx->current_dev, IOCTL_ANALOG_GET, &param) != E_OK) {
        return false;
    }
    b_adc = (uint16_t)param.get.data;

    if (!bsp_encoder_get_mechanical_angle_sample(&mechanical_sample)) {
        return false;
    }

    sample->ia = ((float)a_adc - g_current_a_offset) * FOC_ADC_CODE_TO_CURRENT_SCALE_A_PER_CODE;
    sample->ib = ((float)b_adc - g_current_b_offset) * FOC_ADC_CODE_TO_CURRENT_SCALE_A_PER_CODE;
    sample->mech_angle_deg = mechanical_sample.angle_deg;

    return true;
}

static bool _foc_port_read_mechanical_angle(void *user_data, struct foc_mechanical_sample *sample)
{
    (void)user_data;

    return bsp_encoder_get_mechanical_angle_sample(sample);
}

static uint32_t _foc_port_get_tick_ms(void *user_data)
{
    (void)user_data;

    return (uint32_t)get_ticks_from_isr();
}

static bool _foc_port_write_pwm(void *user_data, const struct foc_pwm_out *pwm)
{
    struct foc_port_ctx *ctx = (struct foc_port_ctx *)user_data;
    union pwmc_ioctl_param param = { 0 };

    if (pwm == NULL) {
        return false;
    }

    if (ctx == NULL) {
        ctx = &g_foc_port_ctx;
    }
    if (ctx->pwm_dev == NULL) {
        return false;
    }

    param.set.channel = PWMC_CHANNEL1;
    param.set.duty = pwm->duty_a;
    if (device_ioctl(ctx->pwm_dev, IOCTL_PWMC_SET_DUTY, &param) != E_OK) {
        return false;
    };

    param.set.channel = PWMC_CHANNEL2;
    param.set.duty = pwm->duty_b;
    if (device_ioctl(ctx->pwm_dev, IOCTL_PWMC_SET_DUTY, &param) != E_OK) {
        return false;
    };

    param.set.channel = PWMC_CHANNEL3;
    param.set.duty = pwm->duty_c;
    if (device_ioctl(ctx->pwm_dev, IOCTL_PWMC_SET_DUTY, &param) != E_OK) {
        return false;
    };

    return true;
}

bool foc_port_init(void)
{
    if ((g_foc_port_ctx.current_dev != NULL) && (g_foc_port_ctx.pwm_dev != NULL)) {
        return true;
    }

    _foc_port_reset_zero_drift_filter(&g_current_a_offset_filter);
    _foc_port_reset_zero_drift_filter(&g_current_b_offset_filter);
    g_current_a_offset = FOC_CURRENT_ADC_MID_CODE;
    g_current_b_offset = FOC_CURRENT_ADC_MID_CODE;

    g_foc_port_ctx.pwm_dev = (device_t *)device_open("motor_pwm");
    g_foc_port_ctx.current_dev = (device_t *)device_open("motor_adc");

    if ((g_foc_port_ctx.current_dev == NULL) || (g_foc_port_ctx.pwm_dev == NULL)) {
        xlog_count("foc_port_init: open device failed adc=%p pwm=%p\r\n",
                   g_foc_port_ctx.current_dev,
                   g_foc_port_ctx.pwm_dev);
        if (g_foc_port_ctx.current_dev != NULL) {
            device_close(g_foc_port_ctx.current_dev);
            g_foc_port_ctx.current_dev = NULL;
        }
        if (g_foc_port_ctx.pwm_dev != NULL) {
            device_close(g_foc_port_ctx.pwm_dev);
            g_foc_port_ctx.pwm_dev = NULL;
        }
        return false;
    }

    return true;
}

const struct foc_config *foc_port_get_config(void)
{
    return &g_foc_config;
}

const struct foc_port *foc_port_get_port(void)
{
    return &g_foc_port;
}

void *foc_port_get_port_ctx(void)
{
    return &g_foc_port_ctx;
}

bool foc_port_register_current_loop_irq(int32_t (*irq_handler)(uint32_t irq, void *args, uint32_t length))
{
    if ((g_foc_port_ctx.current_dev == NULL) || (irq_handler == NULL)) {
        return false;
    }

    return device_ioctl(g_foc_port_ctx.current_dev, IOCTL_ANALOG_SET_IRQ_HANDLER, irq_handler) == E_OK;
}

static bool _foc_port_update_current_zero_drift(void *user_data)
{
    union analog_ioctl_param param = { 0 };
    struct foc_port_ctx *ctx = (struct foc_port_ctx *)user_data;
    uint16_t a_adc = 0U;
    uint16_t b_adc = 0U;

    if (ctx == NULL) {
        ctx = &g_foc_port_ctx;
    }
    if (ctx->current_dev == NULL) {
        return false;
    }

    param.get.channel = 1U;
    if (device_ioctl(ctx->current_dev, IOCTL_ANALOG_GET, &param) != E_OK) {
        return false;
    }
    a_adc = (uint16_t)param.get.data;

    param.get.channel = 2U;
    if (device_ioctl(ctx->current_dev, IOCTL_ANALOG_GET, &param) != E_OK) {
        return false;
    }
    b_adc = (uint16_t)param.get.data;

    g_current_a_offset = low_pass_filter_f(&g_current_a_offset_filter, (float)a_adc);
    g_current_b_offset = low_pass_filter_f(&g_current_b_offset_filter, (float)b_adc);

    return true;
}
/*---------- end of file ----------*/
