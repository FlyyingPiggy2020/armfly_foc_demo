/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : logic_foc_calibration.c
 * @Author       : Codex
 * @Date         : 2026-03-30 11:00:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-30 12:49:13
 * @Brief        : 电角度零位标定逻辑实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "logic_foc_calibration.h"
#include "app_foc.h"
#include <string.h>
/*---------- macro ----------*/
#define LOGIC_FOC_CALIBRATION_ALIGN_ANGLE_DEG    0.0f
#define LOGIC_FOC_CALIBRATION_ALIGN_VOLTAGE_D_PU 0.12f
#define LOGIC_FOC_CALIBRATION_ALIGN_HOLD_MS      500ULL
#define LOGIC_FOC_CALIBRATION_SAMPLE_INTERVAL_MS 10ULL
#define LOGIC_FOC_CALIBRATION_SAMPLE_COUNT       8U
/*---------- type define ----------*/
enum logic_foc_calibration_state {
    LOGIC_FOC_CALIBRATION_STATE_IDLE = 0,
    LOGIC_FOC_CALIBRATION_STATE_ALIGN_PREPARE,
    LOGIC_FOC_CALIBRATION_STATE_ALIGN_HOLD,
    LOGIC_FOC_CALIBRATION_STATE_SAMPLE_OFFSET,
    LOGIC_FOC_CALIBRATION_STATE_DONE,
    LOGIC_FOC_CALIBRATION_STATE_FAILED,
};

struct logic_foc_calibration_ctx {
    enum logic_foc_calibration_state state;
    uint64_t tick_ms;
    uint64_t sample_tick_ms;
    foc_angle_t target_angle_deg;
    foc_angle_t mechanical_angle_avg_deg;
    uint16_t sample_count;
    bool is_init;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static foc_angle_t _logic_foc_calibration_wrap_angle_deg(foc_angle_t angle_deg);
static foc_angle_t _logic_foc_calibration_calc_electrical_zero_offset_deg(
    pmsm_foc_t *foc, foc_angle_t mechanical_angle_deg, foc_angle_t target_angle_deg);
static foc_angle_t _logic_foc_calibration_update_mechanical_angle_avg(
    foc_angle_t current_avg_deg, uint16_t sample_count, foc_angle_t sample_deg);
static void _logic_foc_calibration_enter_failed(const char *reason);
static void _logic_foc_calibration_enter_done(void);
/*---------- variable ----------*/
static struct logic_foc_calibration_ctx g_logic_foc_calibration;
/*---------- function ----------*/
static foc_angle_t _logic_foc_calibration_wrap_angle_deg(foc_angle_t angle_deg)
{
    while (angle_deg >= 360.0f) {
        angle_deg -= 360.0f;
    }
    while (angle_deg < 0.0f) {
        angle_deg += 360.0f;
    }

    return angle_deg;
}

static foc_angle_t _logic_foc_calibration_calc_electrical_zero_offset_deg(
    pmsm_foc_t *foc, foc_angle_t mechanical_angle_deg, foc_angle_t target_angle_deg)
{
    const sensor_profile_t *sensor_profile = NULL;
    const motor_profile_t *motor_profile = NULL;
    foc_angle_t electrical_angle_without_zero = 0.0f;

    if (foc == NULL) {
        return 0.0f;
    }

    sensor_profile = foc->sensor_profile;
    motor_profile = foc->motor_profile;
    if ((motor_profile == NULL) || (sensor_profile == NULL)) {
        return 0.0f;
    }

    electrical_angle_without_zero =
        mechanical_angle_deg * (foc_scalar_t)sensor_profile->angle_direction * (foc_scalar_t)motor_profile->pole_pairs;

    return _logic_foc_calibration_wrap_angle_deg(target_angle_deg - electrical_angle_without_zero);
}

static foc_angle_t _logic_foc_calibration_update_mechanical_angle_avg(
    foc_angle_t current_avg_deg, uint16_t sample_count, foc_angle_t sample_deg)
{
    foc_scalar_t next_count = 0.0f;

    if (sample_count == UINT16_MAX) {
        return current_avg_deg;
    }

    next_count = (foc_scalar_t)(sample_count + 1U);

    return current_avg_deg + ((sample_deg - current_avg_deg) / next_count);
}

static void _logic_foc_calibration_enter_failed(const char *reason)
{
    pmsm_foc_t *foc = app_foc_get_foc();

    if (foc != NULL) {
        pmsm_foc_clear_electrical_zero_offset(foc);
        pmsm_foc_stop(foc);
    }

    g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_FAILED;

    if (reason != NULL) {
        xlog_count("logic_foc_calibration: failed, %s\r\n", reason);
    } else {
        xlog_count("logic_foc_calibration: failed\r\n");
    }
}

static void _logic_foc_calibration_enter_done(void)
{
    pmsm_foc_t *foc = app_foc_get_foc();

    if (foc != NULL) {
        pmsm_foc_stop(foc);
    }

    g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_DONE;
}

bool logic_foc_calibration_init(void)
{
    if (g_logic_foc_calibration.is_init) {
        return true;
    }

    memset(&g_logic_foc_calibration, 0, sizeof(g_logic_foc_calibration));
    g_logic_foc_calibration.target_angle_deg = LOGIC_FOC_CALIBRATION_ALIGN_ANGLE_DEG;
    g_logic_foc_calibration.is_init = true;

    return true;
}

void logic_foc_calibration_process(void)
{
    pmsm_foc_t *foc = app_foc_get_foc();
    foc_dq_t voltage_cmd_dq = { 0 };
    foc_mechanical_angle_sample_t mechanical_sample = { 0 };
    foc_angle_t electrical_zero_offset_deg = 0.0f;
    uint64_t tick_now = get_ticks();

    if ((!g_logic_foc_calibration.is_init) || (foc == NULL)) {
        return;
    }

    switch (g_logic_foc_calibration.state) {
        case LOGIC_FOC_CALIBRATION_STATE_IDLE:
            break;
        case LOGIC_FOC_CALIBRATION_STATE_ALIGN_PREPARE:
            pmsm_foc_stop(foc);
            pmsm_foc_clear_electrical_zero_offset(foc);
            g_logic_foc_calibration.mechanical_angle_avg_deg = 0.0f;
            g_logic_foc_calibration.sample_count = 0U;
            g_logic_foc_calibration.sample_tick_ms = tick_now;
            g_logic_foc_calibration.tick_ms = tick_now;

            voltage_cmd_dq.d = LOGIC_FOC_CALIBRATION_ALIGN_VOLTAGE_D_PU;
            voltage_cmd_dq.q = 0.0f;
            if (!pmsm_foc_apply_voltage_vector(foc, &voltage_cmd_dq, g_logic_foc_calibration.target_angle_deg, true)) {
                _logic_foc_calibration_enter_failed("apply align vector failed");
                break;
            }

            g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_ALIGN_HOLD;
            xlog_count("logic_foc_calibration: align start angle=%.1f deg\r\n",
                       (double)g_logic_foc_calibration.target_angle_deg);
            break;
        case LOGIC_FOC_CALIBRATION_STATE_ALIGN_HOLD:
            if ((tick_now - g_logic_foc_calibration.tick_ms) >= LOGIC_FOC_CALIBRATION_ALIGN_HOLD_MS) {
                g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_SAMPLE_OFFSET;
                g_logic_foc_calibration.sample_tick_ms = tick_now;
            }
            break;
        case LOGIC_FOC_CALIBRATION_STATE_SAMPLE_OFFSET:
            if ((tick_now - g_logic_foc_calibration.sample_tick_ms) < LOGIC_FOC_CALIBRATION_SAMPLE_INTERVAL_MS) {
                break;
            }

            g_logic_foc_calibration.sample_tick_ms = tick_now;
            if (foc->hal_ops.get_mechanical_angle == NULL) {
                _logic_foc_calibration_enter_failed("mechanical angle hal missing");
                break;
            }

            foc->hal_ops.get_mechanical_angle(foc->hal_user_data, (uint32_t)(tick_now * 1000ULL), &mechanical_sample);
            if ((mechanical_sample.status != FOC_ANGLE_STATUS_VALID)
                && (mechanical_sample.status != FOC_ANGLE_STATUS_PREDICTED)
                && (mechanical_sample.status != FOC_ANGLE_STATUS_ESTIMATED)) {
                _logic_foc_calibration_enter_failed("read encoder angle failed");
                break;
            }

            g_logic_foc_calibration.mechanical_angle_avg_deg = _logic_foc_calibration_update_mechanical_angle_avg(
                g_logic_foc_calibration.mechanical_angle_avg_deg,
                g_logic_foc_calibration.sample_count,
                mechanical_sample.mechanical_angle);
            g_logic_foc_calibration.sample_count++;

            if (g_logic_foc_calibration.sample_count < LOGIC_FOC_CALIBRATION_SAMPLE_COUNT) {
                break;
            }

            electrical_zero_offset_deg = _logic_foc_calibration_calc_electrical_zero_offset_deg(
                foc, g_logic_foc_calibration.mechanical_angle_avg_deg, g_logic_foc_calibration.target_angle_deg);
            pmsm_foc_set_electrical_zero_offset(foc, electrical_zero_offset_deg);

            xlog_count("logic_foc_calibration: done mech=%.3f deg zero_offset=%.3f deg\r\n",
                       (double)g_logic_foc_calibration.mechanical_angle_avg_deg,
                       (double)electrical_zero_offset_deg);
            xlog_count("logic_foc_calibration: suggested persisted electrical_zero_offset = %.3f\r\n",
                       (double)electrical_zero_offset_deg);
            _logic_foc_calibration_enter_done();
            break;
        case LOGIC_FOC_CALIBRATION_STATE_DONE:
            g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_IDLE;
            break;
        case LOGIC_FOC_CALIBRATION_STATE_FAILED:
            g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_IDLE;
            break;
        default:
            g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_IDLE;
            break;
    }
}

void logic_foc_calibration_request_start(void)
{
    pmsm_foc_t *foc = app_foc_get_foc();

    if ((!g_logic_foc_calibration.is_init) || (foc == NULL)) {
        return;
    }
    if (g_logic_foc_calibration.state != LOGIC_FOC_CALIBRATION_STATE_IDLE) {
        xlog_count("logic_foc_calibration: already running\r\n");
        return;
    }
    if (foc->runtime.mode != FOC_MODE_STOP) {
        xlog_count("logic_foc_calibration: reject while motor running\r\n");
        return;
    }

    g_logic_foc_calibration.state = LOGIC_FOC_CALIBRATION_STATE_ALIGN_PREPARE;
    g_logic_foc_calibration.tick_ms = get_ticks();
    xlog_count("logic_foc_calibration: start electrical zero calibration\r\n");
}

bool logic_foc_calibration_is_active(void)
{
    return g_logic_foc_calibration.state != LOGIC_FOC_CALIBRATION_STATE_IDLE;
}
/*---------- end of file ----------*/
