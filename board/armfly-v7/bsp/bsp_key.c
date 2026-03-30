/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : bsp_key.c
 * @Author       : Codex
 * @Date         : 2026-03-20 15:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-20 11:23:05
 * @Brief        : 按键板级移植实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "button.h"
#include "stm32h7xx_hal.h"
/*---------- macro ----------*/
#define APP_KEY_PORT_FILTER_TIME     5U
#define APP_KEY_PORT_LONG_TIME       100U
#define APP_KEY_PORT_EVENT_FIFO_SIZE 16U
#define APP_KEY_PORT_LOGIC_KEY_COUNT 3U
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static bool _app_key_port_init(void);
static bool _app_key_port_key1_active(void *ctx);
static bool _app_key_port_key2_active(void *ctx);
static bool _app_key_port_key3_active(void *ctx);
/*---------- variable ----------*/
/* 板级层只描述按键映射关系，消抖和 FIFO 统一由 button 驱动处理。 */
static const struct button_key_cfg s_app_key_port_key_cfgs[APP_KEY_PORT_LOGIC_KEY_COUNT] = {
    { .is_active = _app_key_port_key1_active, .ctx = NULL, .long_time = APP_KEY_PORT_LONG_TIME, .repeat_speed = 0U },
    { .is_active = _app_key_port_key2_active, .ctx = NULL, .long_time = APP_KEY_PORT_LONG_TIME, .repeat_speed = 0U },
    { .is_active = _app_key_port_key3_active, .ctx = NULL, .long_time = APP_KEY_PORT_LONG_TIME, .repeat_speed = 0U },
};

static struct button_key_state s_app_key_port_key_states[APP_KEY_PORT_LOGIC_KEY_COUNT];
static uint32_t s_app_key_port_event_fifo[APP_KEY_PORT_EVENT_FIFO_SIZE];

static button_describe_t s_app_key_port_desc = {
    .number_of_keys = APP_KEY_PORT_LOGIC_KEY_COUNT,
    .filter_time = APP_KEY_PORT_FILTER_TIME,
    .key_cfgs = s_app_key_port_key_cfgs,
    .key_states = s_app_key_port_key_states,
    .event_fifo = s_app_key_port_event_fifo,
    .event_fifo_size = APP_KEY_PORT_EVENT_FIFO_SIZE,
    .ops = {
        .init = _app_key_port_init,
    },
};

DEVICE_DEFINED(app_key, button, &s_app_key_port_desc);
/*---------- function ----------*/
static bool _app_key_port_init(void)
{
    GPIO_InitTypeDef gpio_init = { 0 };

    /* 按键引脚沿用原例程定义：K1=PI8，K2=PC13，K3=PH4。 */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();

    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    gpio_init.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOI, &gpio_init);

    gpio_init.Pin = GPIO_PIN_13;
    HAL_GPIO_Init(GPIOC, &gpio_init);

    gpio_init.Pin = GPIO_PIN_4;
    HAL_GPIO_Init(GPIOH, &gpio_init);

    return true;
}

static bool _app_key_port_key1_active(void *ctx)
{
    (void)ctx;

    /* 原板卡按键为低电平有效。 */
    return (HAL_GPIO_ReadPin(GPIOI, GPIO_PIN_8) == GPIO_PIN_RESET);
}

static bool _app_key_port_key2_active(void *ctx)
{
    (void)ctx;

    /* 原板卡按键为低电平有效。 */
    return (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET);
}

static bool _app_key_port_key3_active(void *ctx)
{
    (void)ctx;

    /* 原板卡按键为低电平有效。 */
    return (HAL_GPIO_ReadPin(GPIOH, GPIO_PIN_4) == GPIO_PIN_RESET);
}
/*---------- end of file ----------*/
