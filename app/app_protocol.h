/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : app_protocol.h
 * @Author       : Codex
 * @Date         : 2026-03-21 09:50:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-24 12:05:00
 * @Brief        : 应用层共享协议定义
 */

#ifndef __APP_PROTOCOL_H__
#define __APP_PROTOCOL_H__

/*---------- includes ----------*/
#include <stdint.h>
#include "app_key.h"
/*---------- macro ----------*/
#define APP_PROTOCOL_KEY_TOPIC   "key/event"
#define APP_PROTOCOL_FOC_SERVICE "foc/service"
/*---------- type define ----------*/
enum app_protocol_foc_service_cmd {
    APP_PROTOCOL_FOC_SERVICE_CMD_CONTROL = 0,
    APP_PROTOCOL_FOC_SERVICE_CMD_GET_REALTIME,
};

enum app_protocol_foc_control_cmd {
    APP_PROTOCOL_FOC_CONTROL_CMD_MOTOR_SWITCH = 0,
    APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_INC,
    APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_DEC,
    APP_PROTOCOL_FOC_CONTROL_CMD_CALIBRATE_ELECTRICAL_ZERO,
};

struct app_protocol_foc_pwm_duty {
    float duty_a;
    float duty_b;
    float duty_c;
};

struct app_protocol_foc_realtime {
    float current_a_real;
    float current_b_real;
    float current_d_pu;
    float current_q_pu;
    float voltage_d_pu;
    float voltage_q_pu;
    float mechanical_angle_deg;
    float electrical_angle_deg;
};

struct app_protocol_foc_service_req {
    enum app_protocol_foc_service_cmd cmd;
    union {
        struct {
            enum app_protocol_foc_control_cmd cmd;
        } control;
    } data;
};

struct app_protocol_foc_service_resp {
    union {
        struct app_protocol_foc_realtime foc_realtime;
    } data;
};

struct app_protocol_key_event {
    app_key_code_t code;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif /* __APP_PROTOCOL_H__ */
