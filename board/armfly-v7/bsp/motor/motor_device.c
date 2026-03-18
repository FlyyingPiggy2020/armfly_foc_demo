/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : motor_device.c
 * @Author       : Codex
 * @Date         : 2026-03-18 10:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-18 10:00:00
 * @Brief        : armfly-v7 FOC 最小设备实例
 */

/*---------- includes ----------*/
#include "options.h"
#include "analog.h"
#include "pwmc.h"
#include "device.h"
/*---------- macro ----------*/
#define MOTOR_ADC_CHANNEL_COUNT 4U
#define MOTOR_PWM_CLOCK_HZ      200000000U
#define MOTOR_PWM_FREQUENCY_HZ  20000U
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
static bool _motor_adc_init(void);
static void _motor_adc_deinit(void);
static bool _motor_adc_enable(bool ctrl);
static uint32_t _motor_adc_get(uint32_t channel);

static bool _motor_pwm_init(void);
static void _motor_pwm_deinit(void);
static bool _motor_pwm_enable(bool ctrl);
static int32_t _motor_pwm_update_precaler_arr(uint32_t precaler, uint32_t arr);
static int32_t _motor_pwm_update_crr(uint8_t channel, uint32_t crr);
/*---------- function prototype ----------*/
/*---------- variable ----------*/
static volatile bool s_motor_adc_enable = false;
static volatile bool s_motor_pwm_enable = false;
static volatile uint32_t s_motor_pwm_prescaler = 0U;
static volatile uint32_t s_motor_pwm_arr = 0U;
static volatile uint32_t s_motor_pwm_ccr[IOCTL_CONFIG_PWMC_CHANNEL] = { 0U };

static analog_describe_t s_motor_adc_desc = {
    .number_of_channels = MOTOR_ADC_CHANNEL_COUNT,
    .ops =
        {
            .init = _motor_adc_init,
            .deinit = _motor_adc_deinit,
            .enable = _motor_adc_enable,
            .get = _motor_adc_get,
            .irq_handler = NULL,
        },
};

static pwmc_describe_t s_motor_pwm_desc = {
    .is_enable = false,
    .clock = MOTOR_PWM_CLOCK_HZ,
    .frequence = MOTOR_PWM_FREQUENCY_HZ,
    .priv =
        {
            .prescaler = 0U,
            .arr = 0U,
            .channel =
                {
                    { .used = true, .duty = 0.0f, .crr = 0U },
                    { .used = true, .duty = 0.0f, .crr = 0U },
                    { .used = true, .duty = 0.0f, .crr = 0U },
                    { .used = false, .duty = 0.0f, .crr = 0U },
                    { .used = false, .duty = 0.0f, .crr = 0U },
                    { .used = false, .duty = 0.0f, .crr = 0U },
                },
        },
    .ops =
        {
            .init = _motor_pwm_init,
            .deinit = _motor_pwm_deinit,
            .enable = _motor_pwm_enable,
            .update_precaler_arr = _motor_pwm_update_precaler_arr,
            .update_crr = _motor_pwm_update_crr,
            .irq_handler = NULL,
        },
};

DEVICE_DEFINED(motor_adc, analog, &s_motor_adc_desc);
DEVICE_DEFINED(motor_pwm, pwmc, &s_motor_pwm_desc);
/*---------- function ----------*/
static bool _motor_adc_init(void)
{
    s_motor_adc_enable = false;
    return true;
}

static void _motor_adc_deinit(void)
{
    s_motor_adc_enable = false;
}

static bool _motor_adc_enable(bool ctrl)
{
    s_motor_adc_enable = ctrl;
    return true;
}

static uint32_t _motor_adc_get(uint32_t channel)
{
    if ((s_motor_adc_enable == false) || (channel >= MOTOR_ADC_CHANNEL_COUNT)) {
        return 0U;
    }

    /* 当前先提供安全占位值，后续替换为真实 ADC 采样。 */
    if (channel == 3U) {
        return 0U;
    }

    return 2048U;
}

static bool _motor_pwm_init(void)
{
    s_motor_pwm_enable = false;
    return true;
}

static void _motor_pwm_deinit(void)
{
    s_motor_pwm_enable = false;
}

static bool _motor_pwm_enable(bool ctrl)
{
    s_motor_pwm_enable = ctrl;
    return true;
}

static int32_t _motor_pwm_update_precaler_arr(uint32_t precaler, uint32_t arr)
{
    s_motor_pwm_prescaler = precaler;
    s_motor_pwm_arr = arr;
    return E_OK;
}

static int32_t _motor_pwm_update_crr(uint8_t channel, uint32_t crr)
{
    if (channel >= IOCTL_CONFIG_PWMC_CHANNEL) {
        return E_WRONG_ARGS;
    }

    s_motor_pwm_ccr[channel] = crr;
    return E_OK;
}
/*---------- end of file ----------*/
