/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : bsp_spi.c
 * @Author       : Codex
 * @Date         : 2026-03-25 11:20:00
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2026-03-26 00:00:00
 * @Brief        : armfly-v7 SPI 板级移植实现
 */

/*---------- includes ----------*/
#include "options.h"
#include "spi_bus.h"
#include "stm32h7xx_hal.h"
#include <string.h>
/*---------- macro ----------*/
#define BSP_SPI_TLE5012B_SCK_PIN         GPIO_PIN_3
#define BSP_SPI_TLE5012B_SCK_PORT        GPIOB
#define BSP_SPI_TLE5012B_SCK_AF          GPIO_AF5_SPI1
#define BSP_SPI_TLE5012B_DATA_PIN        GPIO_PIN_5
#define BSP_SPI_TLE5012B_DATA_PORT       GPIOB
#define BSP_SPI_TLE5012B_DATA_AF         GPIO_AF5_SPI1
#define BSP_SPI_TLE5012B_CS_PIN          GPIO_PIN_10
#define BSP_SPI_TLE5012B_CS_PORT         GPIOG
#define BSP_SPI_TLE5012B_BAUD_PRESCALER  SPI_BAUDRATEPRESCALER_16
#define BSP_SPI_TLE5012B_XFER_TIMEOUT_MS 10U
/*---------- type define ----------*/
struct bsp_spi_ctx {
    SPI_HandleTypeDef hspi;
    volatile uint8_t lock;
    uint8_t bits_per_word;
};
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
static fp_err_t _bsp_spi_init(struct spi_bus_describe *self, struct spi_bus_config *cfg);
static void _bsp_spi_deinit(struct spi_bus_describe *self);
static void _bsp_spi_cs_set(struct spi_bus_describe *self, bool active);
static void _bsp_spi_delay_us(struct spi_bus_describe *self, uint32_t us);
static fp_err_t _bsp_spi_xfer(struct spi_bus_describe *self, struct spi_bus_msg *msgs, uint32_t number);
static fp_err_t
_bsp_spi_tle5012b_read_fast_xfer(struct spi_bus_describe *self, struct bsp_spi_ctx *ctx, struct spi_bus_msg *msgs);
static fp_err_t _bsp_spi_transfer_msg(struct bsp_spi_ctx *ctx, struct spi_bus_msg *msg);
/*---------- variable ----------*/
static struct bsp_spi_ctx s_bsp_spi_tle5012b_ctx = { 0 };

static struct spi_bus_ops s_bsp_spi_ops = {
    .init = _bsp_spi_init,
    .deinit = _bsp_spi_deinit,
    .cs_set = _bsp_spi_cs_set,
    .delay = _bsp_spi_delay_us,
    .xfer = _bsp_spi_xfer,
};

static struct spi_bus_describe s_bsp_spi_tle5012b_desc = {
    .config = {
        .cpol = 0U,
        .cpha = 1U,
        .cs_active_high = 0U,
        .lsb_first = 0U,
        .half_period_us = 0U,
    },
    .ops = &s_bsp_spi_ops,
    .hw_handle = &s_bsp_spi_tle5012b_ctx,
};

DEVICE_DEFINED(spi1_tle5012b, spi_bus, &s_bsp_spi_tle5012b_desc);
/*---------- function ----------*/

static void _bsp_spi_delay_us(struct spi_bus_describe *self, uint32_t us)
{
    (void)self;
    delay_us(us);
}

static void _bsp_spi_cs_set(struct spi_bus_describe *self, bool active)
{
    GPIO_PinState level = GPIO_PIN_SET;

    if ((self != NULL) && (self->config.cs_active_high != 0U)) {
        level = active ? GPIO_PIN_SET : GPIO_PIN_RESET;
    } else {
        level = active ? GPIO_PIN_RESET : GPIO_PIN_SET;
    }

    HAL_GPIO_WritePin(BSP_SPI_TLE5012B_CS_PORT, BSP_SPI_TLE5012B_CS_PIN, level);
}

static fp_err_t
_bsp_spi_tle5012b_read_fast_xfer(struct spi_bus_describe *self, struct bsp_spi_ctx *ctx, struct spi_bus_msg *msgs)
{
    HAL_StatusTypeDef hal_ret = HAL_ERROR;

    if ((self == NULL) || (ctx == NULL) || (msgs == NULL)) {
        return E_WRONG_ARGS;
    }

    if ((ctx->bits_per_word != 16U) || (msgs[0].bits_per_word != ctx->bits_per_word)
        || (msgs[1].bits_per_word != ctx->bits_per_word)) {
        return E_WRONG_ARGS;
    }

    _bsp_spi_cs_set(self, true);

    SPI_1LINE_TX(&ctx->hspi);

    hal_ret = HAL_SPI_Transmit(&ctx->hspi, (uint8_t *)msgs[0].tx_buf, 1U, BSP_SPI_TLE5012B_XFER_TIMEOUT_MS);

    if (hal_ret != HAL_OK) {
        _bsp_spi_cs_set(self, false);
        SPI_1LINE_TX(&ctx->hspi);
        return E_ERROR;
    }

    if (msgs[0].delay_us > 0U) {
        delay_us(msgs[0].delay_us);
    }

    SPI_1LINE_RX(&ctx->hspi);

    hal_ret = HAL_SPI_Receive(&ctx->hspi, (uint8_t *)msgs[1].rx_buf, 2U, BSP_SPI_TLE5012B_XFER_TIMEOUT_MS);

    _bsp_spi_cs_set(self, false);
    SPI_1LINE_TX(&ctx->hspi);

    if (hal_ret != HAL_OK) {
        return E_ERROR;
    }

    return E_OK;
}

static fp_err_t _bsp_spi_transfer_msg(struct bsp_spi_ctx *ctx, struct spi_bus_msg *msg)
{
    HAL_StatusTypeDef hal_ret = HAL_ERROR;

    if ((ctx == NULL) || (msg == NULL)) {
        return E_WRONG_ARGS;
    }

    if ((msg->len == 0U) || ((msg->flags & (SPI_BUS_MSG_FLAG_READ | SPI_BUS_MSG_FLAG_WRITE)) == 0U)) {
        return E_WRONG_ARGS;
    }

    if (((msg->flags & SPI_BUS_MSG_FLAG_READ) != 0U) && ((msg->flags & SPI_BUS_MSG_FLAG_WRITE) != 0U)) {
        return E_WRONG_ARGS;
    }

    if (((msg->flags & SPI_BUS_MSG_FLAG_READ) != 0U) && (msg->rx_buf == NULL)) {
        return E_WRONG_ARGS;
    }

    if (((msg->flags & SPI_BUS_MSG_FLAG_WRITE) != 0U) && (msg->tx_buf == NULL)) {
        return E_WRONG_ARGS;
    }

    if ((msg->bits_per_word != ctx->bits_per_word) || ((msg->bits_per_word != 8U) && (msg->bits_per_word != 16U))) {
        return E_WRONG_ARGS;
    }

    if ((msg->flags & SPI_BUS_MSG_FLAG_WRITE) != 0U) {
        SPI_1LINE_TX(&ctx->hspi);
        hal_ret = HAL_SPI_Transmit(&ctx->hspi, (uint8_t *)msg->tx_buf, msg->len, BSP_SPI_TLE5012B_XFER_TIMEOUT_MS);
    } else {
        SPI_1LINE_RX(&ctx->hspi);
        hal_ret = HAL_SPI_Receive(&ctx->hspi, (uint8_t *)msg->rx_buf, msg->len, BSP_SPI_TLE5012B_XFER_TIMEOUT_MS);
    }

    if (hal_ret != HAL_OK) {
        return E_ERROR;
    }

    return E_OK;
}

static fp_err_t _bsp_spi_xfer(struct spi_bus_describe *self, struct spi_bus_msg *msgs, uint32_t number)
{
    struct bsp_spi_ctx *ctx = NULL;
    fp_err_t err = E_OK;
    bool cs_is_active = false;
    uint32_t msg_index = 0U;

    if ((self == NULL) || (self->hw_handle == NULL) || (msgs == NULL) || (number == 0U)) {
        return E_WRONG_ARGS;
    }

    ctx = (struct bsp_spi_ctx *)self->hw_handle;

    if ((number == 2U) && (msgs[0].tx_buf != NULL) && (msgs[1].rx_buf != NULL) && (msgs[0].len == 1U)
        && (msgs[1].len == 2U) && (msgs[0].bits_per_word == ctx->bits_per_word)
        && (msgs[1].bits_per_word == ctx->bits_per_word)
        && (msgs[0].flags == (SPI_BUS_MSG_FLAG_WRITE | SPI_BUS_MSG_FLAG_KEEP_CS))
        && (msgs[1].flags == SPI_BUS_MSG_FLAG_READ)) {
        return _bsp_spi_tle5012b_read_fast_xfer(self, ctx, msgs);
    }

    for (msg_index = 0U; msg_index < number; msg_index++) {
        struct spi_bus_msg *msg = &msgs[msg_index];

        if (!cs_is_active) {
            _bsp_spi_cs_set(self, true);
            cs_is_active = true;
        }

        err = _bsp_spi_transfer_msg(ctx, msg);
        if (err != E_OK) {
            break;
        }

        if (msg->delay_us > 0U) {
            delay_us(msg->delay_us);
        }

        if ((msg->flags & SPI_BUS_MSG_FLAG_KEEP_CS) == 0U) {
            _bsp_spi_cs_set(self, false);
            cs_is_active = false;
        }
    }

    if (cs_is_active) {
        _bsp_spi_cs_set(self, false);
    }

    SPI_1LINE_TX(&ctx->hspi);
    return err;
}

static fp_err_t _bsp_spi_init(struct spi_bus_describe *self, struct spi_bus_config *cfg)
{
    struct bsp_spi_ctx *ctx = NULL;

    if ((self == NULL) || (cfg == NULL) || (self->hw_handle == NULL)) {
        return E_WRONG_ARGS;
    }

    ctx = (struct bsp_spi_ctx *)self->hw_handle;
    memset(ctx, 0, sizeof(*ctx));

    ctx->hspi.Instance = SPI1;
    ctx->hspi.Init.Mode = SPI_MODE_MASTER;
    ctx->hspi.Init.Direction = SPI_DIRECTION_1LINE;
    ctx->hspi.Init.DataSize = SPI_DATASIZE_16BIT;
    ctx->hspi.Init.CLKPolarity = (cfg->cpol != 0U) ? SPI_POLARITY_HIGH : SPI_POLARITY_LOW;
    ctx->hspi.Init.CLKPhase = (cfg->cpha != 0U) ? SPI_PHASE_2EDGE : SPI_PHASE_1EDGE;
    ctx->hspi.Init.NSS = SPI_NSS_SOFT;
    ctx->hspi.Init.BaudRatePrescaler = BSP_SPI_TLE5012B_BAUD_PRESCALER;
    ctx->hspi.Init.FirstBit = (cfg->lsb_first != 0U) ? SPI_FIRSTBIT_LSB : SPI_FIRSTBIT_MSB;
    ctx->hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    ctx->hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    ctx->hspi.Init.CRCPolynomial = 0x7U;
    ctx->hspi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    ctx->hspi.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    ctx->hspi.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    ctx->hspi.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    ctx->hspi.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    ctx->hspi.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    ctx->hspi.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    ctx->hspi.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    ctx->hspi.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    ctx->hspi.Init.IOSwap = SPI_IO_SWAP_DISABLE;

    if (HAL_SPI_Init(&ctx->hspi) != HAL_OK) {
        return E_ERROR;
    }

    ctx->bits_per_word = 16U;
    _bsp_spi_cs_set(self, false);
    SPI_1LINE_TX(&ctx->hspi);

    return E_OK;
}

static void _bsp_spi_deinit(struct spi_bus_describe *self)
{
    struct bsp_spi_ctx *ctx = NULL;

    if ((self == NULL) || (self->hw_handle == NULL)) {
        return;
    }

    ctx = (struct bsp_spi_ctx *)self->hw_handle;
    _bsp_spi_cs_set(self, false);
    (void)HAL_SPI_DeInit(&ctx->hspi);
    memset(ctx, 0, sizeof(*ctx));
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpio_init = { 0 };
    RCC_PeriphCLKInitTypeDef periph_clk_init = { 0 };

    if ((hspi == NULL) || (hspi->Instance != SPI1)) {
        return;
    }

    periph_clk_init.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
    periph_clk_init.PLL2.PLL2M = 2U;
    periph_clk_init.PLL2.PLL2N = 12U;
    periph_clk_init.PLL2.PLL2P = 2U;
    periph_clk_init.PLL2.PLL2Q = 2U;
    periph_clk_init.PLL2.PLL2R = 2U;
    periph_clk_init.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    periph_clk_init.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
    periph_clk_init.PLL2.PLL2FRACN = 0U;
    periph_clk_init.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&periph_clk_init) != HAL_OK) {
        return;
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    gpio_init.Pin = BSP_SPI_TLE5012B_SCK_PIN;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = BSP_SPI_TLE5012B_SCK_AF;
    HAL_GPIO_Init(BSP_SPI_TLE5012B_SCK_PORT, &gpio_init);

    gpio_init.Pin = BSP_SPI_TLE5012B_DATA_PIN;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = BSP_SPI_TLE5012B_DATA_AF;
    HAL_GPIO_Init(BSP_SPI_TLE5012B_DATA_PORT, &gpio_init);

    gpio_init.Pin = BSP_SPI_TLE5012B_CS_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = 0U;
    HAL_GPIO_Init(BSP_SPI_TLE5012B_CS_PORT, &gpio_init);

    HAL_GPIO_WritePin(BSP_SPI_TLE5012B_CS_PORT, BSP_SPI_TLE5012B_CS_PIN, GPIO_PIN_SET);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if ((hspi == NULL) || (hspi->Instance != SPI1)) {
        return;
    }

    HAL_GPIO_DeInit(BSP_SPI_TLE5012B_SCK_PORT, BSP_SPI_TLE5012B_SCK_PIN);
    HAL_GPIO_DeInit(BSP_SPI_TLE5012B_DATA_PORT, BSP_SPI_TLE5012B_DATA_PIN);
    HAL_GPIO_DeInit(BSP_SPI_TLE5012B_CS_PORT, BSP_SPI_TLE5012B_CS_PIN);
    __HAL_RCC_SPI1_CLK_DISABLE();
}
/*---------- end of file ----------*/
