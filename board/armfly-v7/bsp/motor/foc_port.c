/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : foc_port.c
 * @Author       : Codex
 * @Date         : 2026-03-17 15:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-17 15:46:29
 * @Brief        : armfly-v7 FOC 端口封装实现
 */

/*---------- includes ----------*/
#include "foc_port.h"
#include <string.h>
/*---------- macro ----------*/
#ifndef xlog_cout
#define xlog_cout xlog_count
#endif

#define FOC_PORT_PHASE_CURRENT_DEV_NAME "motor_adc"
#define FOC_PORT_BUS_VOLTAGE_DEV_NAME   "motor_adc"
#define FOC_PORT_PWM_DEV_NAME           "motor_pwm"
#define FOC_PORT_ANGLE_DEV_NAME         "motor_angle"

#define FOC_PORT_PHASE_CURRENT_A_CH     0U
#define FOC_PORT_PHASE_CURRENT_B_CH     1U
#define FOC_PORT_PHASE_CURRENT_C_CH     2U
#define FOC_PORT_BUS_VOLTAGE_CH         3U

#define FOC_PORT_PWM_A_CH               PWMC_CHANNEL1
#define FOC_PORT_PWM_B_CH               PWMC_CHANNEL2
#define FOC_PORT_PWM_C_CH               PWMC_CHANNEL3
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static void _foc_port_load_default_profiles(foc_port_t *port);
static bool _foc_port_read_analog_channel(device_t *dev, uint32_t channel, uint32_t *value);
static bool _foc_port_convert_phase_current(foc_port_t *port,
                                            const uint32_t raw_phase_current[3],
                                            uint8_t channel_count,
                                            foc_abc_t *phase_current);
static foc_scalar_t _foc_port_convert_bus_voltage(foc_port_t *port, uint32_t raw_bus_voltage);
static void _foc_port_read_angle(foc_port_t *port,
                                 device_t *angle_dev,
                                 uint32_t target_tick_us,
                                 foc_angle_sample_t *sample);
static void _foc_port_read_current_loop_sample(void *user_data, foc_current_loop_sample_t *sample);
static void _foc_port_get_electrical_angle(void *user_data, uint32_t target_tick_us, foc_angle_sample_t *sample);
static void _foc_port_write_pwm_duty(void *user_data, const foc_pwm_duty_t *duty);
static void _foc_port_set_output_enable(void *user_data, bool enable);
static uint32_t _foc_port_get_tick_us(void *user_data);
/*---------- variable ----------*/
/*---------- function ----------*/
static void _foc_port_load_default_profiles(foc_port_t *port)
{
    /* 当前先收一套板级默认参数，后续再替换成真实电机和驱动板数据 */
    memset(&port->motor_profile, 0, sizeof(port->motor_profile));
    memset(&port->power_stage_profile, 0, sizeof(port->power_stage_profile));
    memset(&port->sensor_profile, 0, sizeof(port->sensor_profile));
    memset(&port->ctrl_cfg, 0, sizeof(port->ctrl_cfg));

    port->motor_profile.pole_pairs = 7U;
    port->motor_profile.electrical_direction = 1;
    port->motor_profile.current_limit = 10.0f;
    port->motor_profile.speed_limit_rpm = 3000.0f;

    port->power_stage_profile.pwm_frequency_hz = 20000U;
    port->power_stage_profile.deadtime_ns = 500U;
    port->power_stage_profile.adc_full_scale = 4095U;
    port->power_stage_profile.adc_ref_voltage = 3.3f;
    port->power_stage_profile.shunt_resistor_ohm = 0.01f;
    port->power_stage_profile.current_sense_gain = 20.0f;
    port->power_stage_profile.bus_voltage_ratio = 11.0f;
    port->power_stage_profile.over_current_limit = 15.0f;

    port->sensor_profile.phase_current_channel_count = FOC_PORT_PHASE_CURRENT_CHANNEL_COUNT;
    port->sensor_profile.encoder_direction = 1;

    port->ctrl_cfg.id_kp = 0.1f;
    port->ctrl_cfg.id_ki = 0.01f;
    port->ctrl_cfg.iq_kp = 0.1f;
    port->ctrl_cfg.iq_ki = 0.01f;
    port->ctrl_cfg.speed_kp = 0.01f;
    port->ctrl_cfg.speed_ki = 0.001f;
    port->ctrl_cfg.id_limit = 5.0f;
    port->ctrl_cfg.iq_limit = 5.0f;
    port->ctrl_cfg.voltage_limit = 0.95f;
    port->ctrl_cfg.current_loop_hz = port->power_stage_profile.pwm_frequency_hz;
    port->ctrl_cfg.speed_loop_hz = 1000U;
}

static bool _foc_port_read_analog_channel(device_t *dev, uint32_t channel, uint32_t *value)
{
    union analog_ioctl_param param = { 0 };

    if ((dev == NULL) || (value == NULL)) {
        return false;
    }

    param.get.channel = channel;
    if (device_ioctl(dev, IOCTL_ANALOG_GET, &param) != E_OK) {
        return false;
    }

    *value = param.get.data;
    return true;
}

static bool _foc_port_convert_phase_current(foc_port_t *port,
                                            const uint32_t raw_phase_current[3],
                                            uint8_t channel_count,
                                            foc_abc_t *phase_current)
{
    foc_scalar_t adc_lsb_voltage = 0.0f;
    foc_scalar_t current_scale = 0.0f;
    foc_scalar_t raw_current[3] = { 0.0f };
    uint8_t i = 0;

    if ((port == NULL) || (phase_current == NULL) || (channel_count == 0U)) {
        return false;
    }

    adc_lsb_voltage =
        port->power_stage_profile.adc_ref_voltage / (foc_scalar_t)port->power_stage_profile.adc_full_scale;
    current_scale = adc_lsb_voltage
                    / (port->power_stage_profile.shunt_resistor_ohm * port->power_stage_profile.current_sense_gain);

    for (i = 0; i < channel_count; ++i) {
        raw_current[i] = (foc_scalar_t)raw_phase_current[i] * current_scale;
    }

    phase_current->a = raw_current[0] - port->sensor_profile.current_a_offset;
    phase_current->b = raw_current[1] - port->sensor_profile.current_b_offset;
    if (channel_count >= 3U) {
        phase_current->c = raw_current[2] - port->sensor_profile.current_c_offset;
    } else {
        phase_current->c = -(phase_current->a + phase_current->b);
    }

    return true;
}

static foc_scalar_t _foc_port_convert_bus_voltage(foc_port_t *port, uint32_t raw_bus_voltage)
{
    foc_scalar_t adc_lsb_voltage = 0.0f;

    if (port == NULL) {
        return 0.0f;
    }

    adc_lsb_voltage =
        port->power_stage_profile.adc_ref_voltage / (foc_scalar_t)port->power_stage_profile.adc_full_scale;
    return (foc_scalar_t)raw_bus_voltage * adc_lsb_voltage * port->power_stage_profile.bus_voltage_ratio;
}

static void _foc_port_read_angle(foc_port_t *port,
                                 device_t *angle_dev,
                                 uint32_t target_tick_us,
                                 foc_angle_sample_t *sample)
{
    (void)port;
    (void)angle_dev;

    if (sample == NULL) {
        return;
    }

    /* 当前保留角度读取占位，后续在这里接具体磁编、霍尔或估算器 */
    sample->electrical_angle = 0U;
    sample->electrical_speed = 0.0f;
    sample->angle_tick_us = target_tick_us;
    sample->status = FOC_ANGLE_STATUS_NONE;
}

static void _foc_port_read_current_loop_sample(void *user_data, foc_current_loop_sample_t *sample)
{
    foc_port_t *port = (foc_port_t *)user_data;
    uint32_t raw_phase_current[3] = { 0 };
    uint32_t raw_bus_voltage = 0;

    if ((port == NULL) || (sample == NULL)) {
        return;
    }

    memset(sample, 0, sizeof(*sample));

    (void)_foc_port_read_analog_channel(port->phase_current_dev, FOC_PORT_PHASE_CURRENT_A_CH, &raw_phase_current[0]);
    (void)_foc_port_read_analog_channel(port->phase_current_dev, FOC_PORT_PHASE_CURRENT_B_CH, &raw_phase_current[1]);
    (void)_foc_port_read_analog_channel(port->bus_voltage_dev, FOC_PORT_BUS_VOLTAGE_CH, &raw_bus_voltage);

    (void)_foc_port_convert_phase_current(port, raw_phase_current, FOC_PORT_PHASE_CURRENT_CHANNEL_COUNT, &sample->phase_current);
    sample->bus_voltage = _foc_port_convert_bus_voltage(port, raw_bus_voltage);
    sample->sample_tick_us = _foc_port_get_tick_us(port);
}

static void _foc_port_get_electrical_angle(void *user_data, uint32_t target_tick_us, foc_angle_sample_t *sample)
{
    foc_port_t *port = (foc_port_t *)user_data;

    if (sample == NULL) {
        return;
    }

    memset(sample, 0, sizeof(*sample));
    sample->status = FOC_ANGLE_STATUS_NONE;

    if (port == NULL) {
        return;
    }

    _foc_port_read_angle(port, port->angle_dev, target_tick_us, sample);
}

static void _foc_port_write_pwm_duty(void *user_data, const foc_pwm_duty_t *duty)
{
    foc_port_t *port = (foc_port_t *)user_data;
    union pwmc_ioctl_param param = { 0 };
    foc_scalar_t duty_value[3];
    uint8_t i = 0;

    if ((port == NULL) || (port->pwm_dev == NULL) || (duty == NULL)) {
        return;
    }

    duty_value[0] = duty->duty_a;
    duty_value[1] = duty->duty_b;
    duty_value[2] = duty->duty_c;

    for (i = 0; i < 3; ++i) {
        memset(&param, 0, sizeof(param));
        param.set.channel = (i == 0U) ? FOC_PORT_PWM_A_CH : ((i == 1U) ? FOC_PORT_PWM_B_CH : FOC_PORT_PWM_C_CH);
        param.set.duty = foc_scalar_to_float(duty_value[i]);
        (void)device_ioctl(port->pwm_dev, IOCTL_PWMC_SET_DUTY, &param);
    }
}

static void _foc_port_set_output_enable(void *user_data, bool enable)
{
    foc_port_t *port = (foc_port_t *)user_data;

    if ((port == NULL) || (port->pwm_dev == NULL)) {
        return;
    }

    (void)device_ioctl(port->pwm_dev, enable ? IOCTL_PWMC_ENABLE : IOCTL_PWMC_DISABLE, NULL);
}

static uint32_t _foc_port_get_tick_us(void *user_data)
{
    (void)user_data;

    /* 当前先使用系统毫秒 tick 扩展为微秒占位，后续替换为真实高精度时间基 */
    return get_ticks() * 1000U;
}

bool foc_port_init(foc_port_t *port)
{
    if (port == NULL) {
        xlog_cout("foc_port_init: port is null\r\n");
        return false;
    }

    memset(port, 0, sizeof(*port));
    _foc_port_load_default_profiles(port);

    if (!motor_profile_is_valid(&port->motor_profile) || !power_stage_profile_is_valid(&port->power_stage_profile)
        || !sensor_profile_is_valid(&port->sensor_profile) || !foc_ctrl_cfg_is_valid(&port->ctrl_cfg)) {
        xlog_cout("foc_port_init: profile invalid\r\n");
        return false;
    }

    port->phase_current_dev = (device_t *)device_open((char *)FOC_PORT_PHASE_CURRENT_DEV_NAME);
    port->bus_voltage_dev = (device_t *)device_open((char *)FOC_PORT_BUS_VOLTAGE_DEV_NAME);
    port->pwm_dev = (device_t *)device_open((char *)FOC_PORT_PWM_DEV_NAME);
    port->angle_dev = (device_t *)device_open((char *)FOC_PORT_ANGLE_DEV_NAME);

    if ((port->phase_current_dev == NULL) || (port->bus_voltage_dev == NULL) || (port->pwm_dev == NULL)) {
        xlog_cout("foc_port_init: device open failed adc=%p bus=%p pwm=%p angle=%p\r\n",
                  port->phase_current_dev,
                  port->bus_voltage_dev,
                  port->pwm_dev,
                  port->angle_dev);
        foc_port_deinit(port);
        return false;
    }

    (void)device_ioctl(port->phase_current_dev, IOCTL_ANALOG_ENABLE, NULL);
    if (port->bus_voltage_dev != port->phase_current_dev) {
        (void)device_ioctl(port->bus_voltage_dev, IOCTL_ANALOG_ENABLE, NULL);
    }
    (void)device_ioctl(port->pwm_dev, IOCTL_PWMC_DISABLE, NULL);

    port->hal_ops.read_current_loop_sample = _foc_port_read_current_loop_sample;
    port->hal_ops.get_electrical_angle = _foc_port_get_electrical_angle;
    port->hal_ops.write_pwm_duty = _foc_port_write_pwm_duty;
    port->hal_ops.set_output_enable = _foc_port_set_output_enable;
    port->hal_ops.get_tick_us = _foc_port_get_tick_us;

    if (!pmsm_foc_init(&port->foc,
                       &port->motor_profile,
                       &port->power_stage_profile,
                       &port->sensor_profile,
                       &port->ctrl_cfg,
                       &port->hal_ops,
                       port)) {
        xlog_cout("foc_port_init: pmsm_foc_init failed\r\n");
        return false;
    }

    return true;
}

void foc_port_deinit(foc_port_t *port)
{
    if (port == NULL) {
        return;
    }

    pmsm_foc_stop(&port->foc);

    if (port->angle_dev != NULL) {
        device_close(port->angle_dev);
        port->angle_dev = NULL;
    }
    if (port->pwm_dev != NULL) {
        (void)device_ioctl(port->pwm_dev, IOCTL_PWMC_DISABLE, NULL);
        device_close(port->pwm_dev);
        port->pwm_dev = NULL;
    }
    if ((port->bus_voltage_dev != NULL) && (port->bus_voltage_dev != port->phase_current_dev)) {
        (void)device_ioctl(port->bus_voltage_dev, IOCTL_ANALOG_DISABLE, NULL);
    }
    if (port->bus_voltage_dev != NULL) {
        device_close(port->bus_voltage_dev);
        port->bus_voltage_dev = NULL;
    }
    if (port->phase_current_dev != NULL) {
        (void)device_ioctl(port->phase_current_dev, IOCTL_ANALOG_DISABLE, NULL);
        device_close(port->phase_current_dev);
        port->phase_current_dev = NULL;
    }

    memset(&port->foc, 0, sizeof(port->foc));
    memset(&port->hal_ops, 0, sizeof(port->hal_ops));
}

pmsm_foc_t *foc_port_get_foc(foc_port_t *port)
{
    if (port == NULL) {
        return NULL;
    }

    return &port->foc;
}
/*---------- end of file ----------*/
