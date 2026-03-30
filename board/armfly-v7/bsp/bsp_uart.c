/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : bsp_uart.c
 * @Author       : Codex
 * @Date         : 2026-03-24 10:40:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-24 10:40:00
 * @Brief        : armfly-v7 串口板级移植实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "dev_uart.h"
#include "stm32h7xx_hal.h"
/*---------- macro ----------*/
#define BSP_UART1_RX_BUF_SIZE 256U
#define BSP_UART1_TX_BUF_SIZE 256U
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
UART_HandleTypeDef huart1;
/*---------- function prototype ----------*/
static fp_err_t _bsp_uart_init(struct dev_uart_describe *self, struct dev_uart_config *cfg);
static void _bsp_uart_deinit(struct dev_uart_describe *self);
static fp_err_t _bsp_uart_control(struct dev_uart_describe *self, uint32_t cmd, void *arg);
static void _bsp_uart_start_transmit(struct dev_uart_describe *self, uint8_t *buf, uint32_t len);
static fp_err_t _bsp_uart_start_receive(struct dev_uart_describe *self, uint8_t *buf, uint32_t len);
static void _bsp_uart_apply_config(UART_HandleTypeDef *huart, const struct dev_uart_config *cfg);
/*---------- variable ----------*/
static struct dev_uart_ops s_bsp_uart_ops = {
    .init = _bsp_uart_init,
    .deinit = _bsp_uart_deinit,
    .control = _bsp_uart_control,
    .start_transmit = _bsp_uart_start_transmit,
    .start_receive = _bsp_uart_start_receive,
};

static struct dev_uart_describe s_bsp_usart1_desc = {
    .config =
        {
            .baudrate = 921600U,
            .databits = 8U,
            .stopbits = 1U,
            .parity = 0U,
            .bufsz_rx = BSP_UART1_RX_BUF_SIZE,
            .bufsz_tx = BSP_UART1_TX_BUF_SIZE,
            .flags = DEV_UART_FLAG_PKT_MODE,
            .rx_mode = UART_RX_MODE_STANDARD,
            .tx_mode = UART_TX_MODE_ASYNC,
        },
    .ops = &s_bsp_uart_ops,
    .hw_handle = &huart1,
};

DEVICE_DEFINED(usart1, dev_uart, &s_bsp_usart1_desc);
/*---------- function ----------*/
static void _bsp_uart_apply_config(UART_HandleTypeDef *huart, const struct dev_uart_config *cfg)
{
    huart->Init.BaudRate = cfg->baudrate;
    huart->Init.WordLength = (cfg->databits == 9U) ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B;
    huart->Init.StopBits = (cfg->stopbits == 2U) ? UART_STOPBITS_2 : UART_STOPBITS_1;

    switch (cfg->parity) {
        case 1U:
            huart->Init.Parity = UART_PARITY_ODD;
            break;
        case 2U:
            huart->Init.Parity = UART_PARITY_EVEN;
            break;
        default:
            huart->Init.Parity = UART_PARITY_NONE;
            break;
    }

    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart->Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    huart->FifoMode = UART_FIFOMODE_DISABLE;
}

static fp_err_t _bsp_uart_init(struct dev_uart_describe *self, struct dev_uart_config *cfg)
{
    UART_HandleTypeDef *huart = NULL;

    if ((self == NULL) || (cfg == NULL) || (self->hw_handle == NULL)) {
        return E_WRONG_ARGS;
    }

    huart = (UART_HandleTypeDef *)self->hw_handle;
    huart->Instance = USART1;

    _bsp_uart_apply_config(huart, cfg);

    if (HAL_UART_Init(huart) != HAL_OK) {
        return E_ERROR;
    }

    if (HAL_UARTEx_DisableFifoMode(huart) != HAL_OK) {
        return E_ERROR;
    }

    return E_OK;
}

static void _bsp_uart_deinit(struct dev_uart_describe *self)
{
    if ((self == NULL) || (self->hw_handle == NULL)) {
        return;
    }

    HAL_UART_DeInit((UART_HandleTypeDef *)self->hw_handle);
}

static fp_err_t _bsp_uart_control(struct dev_uart_describe *self, uint32_t cmd, void *arg)
{
    (void)self;
    (void)cmd;
    (void)arg;

    return E_OK;
}

static void _bsp_uart_start_transmit(struct dev_uart_describe *self, uint8_t *buf, uint32_t len)
{
    if ((self == NULL) || (self->hw_handle == NULL) || (buf == NULL) || (len == 0U)) {
        return;
    }

    (void)HAL_UART_Transmit_IT((UART_HandleTypeDef *)self->hw_handle, buf, (uint16_t)len);
}

static fp_err_t _bsp_uart_start_receive(struct dev_uart_describe *self, uint8_t *buf, uint32_t len)
{
    if ((self == NULL) || (self->hw_handle == NULL) || (buf == NULL) || (len == 0U)) {
        return E_WRONG_ARGS;
    }

    if (HAL_UARTEx_ReceiveToIdle_IT((UART_HandleTypeDef *)self->hw_handle, buf, (uint16_t)len) != HAL_OK) {
        return E_ERROR;
    }

    return E_OK;
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init = { 0 };
    RCC_PeriphCLKInitTypeDef periph_clk_init = { 0 };

    if (huart->Instance != USART1) {
        return;
    }

    periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    periph_clk_init.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&periph_clk_init) != HAL_OK) {
        return;
    }

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    gpio_init.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    HAL_NVIC_SetPriority(USART1_IRQn, 5U, 0U);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) {
        return;
    }

    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    __HAL_RCC_USART1_CLK_DISABLE();
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) {
        return;
    }

    device_irq_process(&device_usart1, DEV_UART_EVENT_TX_COMPLETE, NULL, 0U);
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart->Instance != USART1) {
        return;
    }

    device_irq_process(&device_usart1, DEV_UART_EVENT_RX_COMPLETE, NULL, size);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != USART1) {
        return;
    }

    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);
    device_irq_process(&device_usart1, DEV_UART_EVENT_RX_ERROR, NULL, 0U);
}
/*---------- end of file ----------*/
