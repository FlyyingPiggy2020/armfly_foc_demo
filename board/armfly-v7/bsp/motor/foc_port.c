/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_port.c
 * @Author       : Codex
 * @Date         : 2026-03-17 15:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 09:18:43
 * @Brief        : armfly-v7 FOC 端口封装实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "foc_port.h"
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static void _foc_port_read_current_loop_sample(void *user_data, foc_current_loop_sample_t *sample);
static void _foc_port_get_electrical_angle(void *user_data, uint32_t target_tick_us, foc_angle_sample_t *sample);
static void _foc_port_write_pwm_duty(void *user_data, const foc_pwm_duty_t *duty);
static void _foc_port_set_output_enable(void *user_data, bool enable);
static uint32_t _foc_port_get_tick_us(void *user_data);
/*---------- variable ----------*/
static motor_profile_t g_motor_profile = {
    .pole_pairs = 7U,
    .electrical_direction = 1,
    .current_limit = 10.0f,
    .speed_limit_rpm = 3000.0f,
};
static power_stage_profile_t g_power_stage_profile = {
    .pwm_frequency_hz = 8000U,
    .deadtime_ns = 1500U,
    .adc_full_scale = 4095U,
    .adc_ref_voltage = 3.3f,
    .shunt_resistor_ohm = 0.01f,
    .current_sense_gain = 20.0f,
    .bus_voltage_ratio = 11.0f,
    .over_current_limit = 15.0f,
};
static sensor_profile_t g_sensor_profile = {
    .phase_current_channel_count = 2U,
    .encoder_direction = 1,
};
static foc_ctrl_cfg_t g_ctrl_cfg = {
    .id_kp = 0.1f,
    .id_ki = 0.01f,
    .iq_kp = 0.1f,
    .iq_ki = 0.01f,
    .speed_kp = 0.01f,
    .speed_ki = 0.001f,
    .id_limit = 5.0f,
    .iq_limit = 5.0f,
    .voltage_limit = 0.95f,
    .current_loop_hz = 8000U,
    .speed_loop_hz = 800U,
};

static const foc_hal_ops_t g_foc_port_hal_ops = {
    .read_current_loop_sample = _foc_port_read_current_loop_sample,
    .get_electrical_angle = _foc_port_get_electrical_angle,
    .write_pwm_duty = _foc_port_write_pwm_duty,
    .set_output_enable = _foc_port_set_output_enable,
    .get_tick_us = _foc_port_get_tick_us,
};
/*---------- function ----------*/
static void _foc_port_read_current_loop_sample(void *user_data, foc_current_loop_sample_t *sample)
{
    (void)user_data;

    if (sample == NULL) {
        return;
    }

    *sample = (foc_current_loop_sample_t){ 0 };
}

static void _foc_port_get_electrical_angle(void *user_data, uint32_t target_tick_us, foc_angle_sample_t *sample)
{
    (void)user_data;
    (void)target_tick_us;

    if (sample == NULL) {
        return;
    }

    *sample = (foc_angle_sample_t){ 0 };
    sample->status = FOC_ANGLE_STATUS_NONE;
}

static void _foc_port_write_pwm_duty(void *user_data, const foc_pwm_duty_t *duty)
{
    (void)user_data;
    (void)duty;
}

static void _foc_port_set_output_enable(void *user_data, bool enable)
{
    (void)user_data;
    (void)enable;
}

static uint32_t _foc_port_get_tick_us(void *user_data)
{
    (void)user_data;
    return 0U;
}

bool foc_port_init(void)
{
    if (!motor_profile_is_valid(&g_motor_profile) || !power_stage_profile_is_valid(&g_power_stage_profile)
        || !sensor_profile_is_valid(&g_sensor_profile) || !foc_ctrl_cfg_is_valid(&g_ctrl_cfg)) {
        xlog_count("foc_port_init: profile invalid\r\n");
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
    return NULL;
}
/*---------- end of file ----------*/
