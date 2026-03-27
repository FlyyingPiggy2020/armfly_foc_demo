/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : bsp_foc.c
 * @Author       : Codex
 * @Date         : 2026-03-17 15:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-23 16:18:24
 * @Brief        : armfly-v7 FOC 端口封装实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "cpu.h"
#include "bsp_encoder.h"
#include "bsp_foc.h"
#include "analog.h"
#include "pmsm_foc.h"
#include "pwmc.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static void _foc_port_read_current_loop_sample(void *user_data, foc_current_loop_sample_t *sample);
static void _foc_port_get_electrical_angle(void *user_data, uint32_t target_tick_us, foc_angle_sample_t *sample);
static void _foc_port_write_pwm_duty(void *user_data, const foc_pwm_duty_t *duty);
static void _foc_port_set_output_enable(void *user_data, bool enable);
static bool _foc_port_read_adc_channel(struct foc_port_ctx *ctx, uint32_t channel, uint16_t *adc_code);
static bool _foc_port_write_pwm_channel(struct foc_port_ctx *ctx, uint8_t channel, foc_scalar_t duty);
/*---------- variable ----------*/
static motor_profile_t g_motor_profile = {
    .pole_pairs = 7U,
    .electrical_direction = 1,
    .inv_i_base = 1,
    .inv_v_base = 0.144337f,
};
static power_stage_profile_t g_power_stage_profile = {
    .pwm_frequency_hz = 8000,
    .bus_voltage_ratio = 11.0f,
    .over_current_limit = 15.0f,
};
static sensor_profile_t g_sensor_profile = {
    .encoder_cpr = 32768U,
    .encoder_direction = 1,
};
static foc_ctrl_cfg_t g_ctrl_cfg = {
    .id_kp_pu = 0.5079f,
    .id_ki_pu = 0.1156f,
    .iq_kp_pu = 0.5079f,
    .iq_ki_pu = 0.1156f,
    .voltage_limit_pu = 0.95f, // 电压限幅_PU值(需要给采样留时间，所以不能全功率1.0输出)
    .current_loop_hz = 200,
    .speed_loop_hz = 20,
};

static const foc_hal_ops_t g_foc_port_hal_ops = {
    .read_current_loop_sample = _foc_port_read_current_loop_sample,
    .get_electrical_angle = _foc_port_get_electrical_angle,
    .write_pwm_duty = _foc_port_write_pwm_duty,
    .set_output_enable = _foc_port_set_output_enable,
};
static struct foc_port_ctx g_foc_port_ctx = { 0 };
/*---------- function ----------*/
static bool _foc_port_read_adc_channel(struct foc_port_ctx *ctx, uint32_t channel, uint16_t *adc_code)
{
    union analog_ioctl_param param = { 0 };

    if ((ctx == NULL) || (ctx->current_dev == NULL) || (adc_code == NULL)) {
        return false;
    }

    param.get.channel = channel;
    if (device_ioctl(ctx->current_dev, IOCTL_ANALOG_GET, &param) != E_OK) {
        return false;
    }

    *adc_code = (uint16_t)param.get.data;

    return true;
}

static bool _foc_port_write_pwm_channel(struct foc_port_ctx *ctx, uint8_t channel, foc_scalar_t duty)
{
    union pwmc_ioctl_param param = { 0 };

    if ((ctx == NULL) || (ctx->pwm_dev == NULL)) {
        return false;
    }

    if (duty < 0.0f) {
        duty = 0.0f;
    } else if (duty > 1.0f) {
        duty = 1.0f;
    }

    param.set.channel = channel;
    param.set.duty = duty;

    return device_ioctl(ctx->pwm_dev, IOCTL_PWMC_SET_DUTY, &param) == E_OK;
}

/**
 * @brief 读取a,b,c三相电流的采样值
 * @param {void} *user_data
 * @param {foc_current_loop_sample_t} *sample
 * @return {*}
 */
static void _foc_port_read_current_loop_sample(void *user_data, foc_current_loop_sample_t *sample)
{
    uint16_t a_adc, b_adc, c_adc;
    struct foc_port_ctx *ctx = (struct foc_port_ctx *)user_data;

    if (sample == NULL) {
        return;
    }

    *sample = (foc_current_loop_sample_t){ 0 };

    if (ctx == NULL) {
        ctx = &g_foc_port_ctx;
    }
    if (ctx->current_dev == NULL) {
        return;
    }

    if (!_foc_port_read_adc_channel(ctx, 0U, &a_adc)) {
        return;
    }
    if (!_foc_port_read_adc_channel(ctx, 1U, &b_adc)) {
        return;
    }
    if (!_foc_port_read_adc_channel(ctx, 2U, &c_adc)) {
        return;
    }

    // (foc_scalar_t)(a_adc - 32767.0f) / 65535.0f * 3.3f / 0.010 * 100
    sample->a_real = (foc_scalar_t)(a_adc - 32767.0f) * 5.0354772e-5;
    sample->b_real = (foc_scalar_t)(b_adc - 32767.0f) * 5.0354772e-5;
    sample->bus_voltage = 12.0f;
    sample->sample_tick_us = (uint32_t)(tick_get_from_isr() * 1000ULL);

}

static void _foc_port_get_electrical_angle(void *user_data, uint32_t target_tick_us, foc_angle_sample_t *sample)
{
    static float angle = 0;

    if (sample == NULL) {
        return;
    }
    (void)user_data;

    /* 电流环在消费点实时拉取磁编角度，读取失败时再退回调试兜底角度。 */
    bsp_encoder_get_angle_sample(target_tick_us, sample);
}

static void _foc_port_write_pwm_duty(void *user_data, const foc_pwm_duty_t *duty)
{
    struct foc_port_ctx *ctx = (struct foc_port_ctx *)user_data;

    if (duty == NULL) {
        return;
    }

    if (ctx == NULL) {
        ctx = &g_foc_port_ctx;
    }
    if (ctx->pwm_dev == NULL) {
        return;
    }

    if (!_foc_port_write_pwm_channel(ctx, PWMC_CHANNEL1, duty->duty_a)) {
        return;
    }
    if (!_foc_port_write_pwm_channel(ctx, PWMC_CHANNEL2, duty->duty_b)) {
        return;
    }
    (void)_foc_port_write_pwm_channel(ctx, PWMC_CHANNEL3, duty->duty_c);
}

static void _foc_port_set_output_enable(void *user_data, bool enable)
{
    (void)user_data;
    (void)enable;
}

bool foc_port_init(void)
{
    if (!motor_profile_is_valid(&g_motor_profile) || !power_stage_profile_is_valid(&g_power_stage_profile)
        || !sensor_profile_is_valid(&g_sensor_profile) || !foc_ctrl_cfg_is_valid(&g_ctrl_cfg)) {
        xlog_count("foc_port_init: profile invalid\r\n");
        return false;
    }

    if ((g_foc_port_ctx.current_dev != NULL) && (g_foc_port_ctx.pwm_dev != NULL)) {
        return true;
    }
    
    // 必须先打开定时器，再初始化ADC；因为ADC的采样源触发源是定时器的事件，如果先后顺序搞反，则无法进行ADC采集
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

const motor_profile_t *foc_port_get_motor_profile(void)
{
    return &g_motor_profile;
}

const power_stage_profile_t *foc_port_get_power_stage_profile(void)
{
    return &g_power_stage_profile;
}

const sensor_profile_t *foc_port_get_sensor_profile(void)
{
    return &g_sensor_profile;
}

const foc_ctrl_cfg_t *foc_port_get_ctrl_cfg(void)
{
    return &g_ctrl_cfg;
}

const foc_hal_ops_t *foc_port_get_hal_ops(void)
{
    return &g_foc_port_hal_ops;
}

void *foc_port_get_hal_user_data(void)
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
/*---------- end of file ----------*/
