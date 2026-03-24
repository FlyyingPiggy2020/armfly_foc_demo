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
    APP_PROTOCOL_FOC_SERVICE_CMD_GET_PWM_DUTY,
};

enum app_protocol_foc_control_cmd {
    APP_PROTOCOL_FOC_CONTROL_CMD_TOGGLE = 0,
    APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_UP,
    APP_PROTOCOL_FOC_CONTROL_CMD_SPEED_DOWN,
};

struct app_protocol_foc_pwm_duty {
    float duty_a;
    float duty_b;
    float duty_c;
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
        struct app_protocol_foc_pwm_duty pwm_duty;
    } data;
};

struct app_protocol_key_event {
    app_key_code_t code;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#endif /* __APP_PROTOCOL_H__ */
