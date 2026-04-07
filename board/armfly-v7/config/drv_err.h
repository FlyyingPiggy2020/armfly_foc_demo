/*
 * Copyright (c) 2026 by Lu Xianfan.
 * @FilePath     : drv_err.h
 * @Author       : Codex
 * @Date         : 2026-03-17 17:00:00
 * @LastEditors  : Codex
 * @LastEditTime : 2026-03-17 17:00:00
 * @Brief        : fp-sdk 驱动错误码兼容映射
 */

#ifndef __BOARD_DRV_ERR_H__
#define __BOARD_DRV_ERR_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
#include "errorno.h"
#include "misc.h"
/*---------- macro ----------*/
#ifndef ASSERT
#define ASSERT(expr)    \
    do {                \
        if (!(expr)) {  \
            while (1) { \
            }           \
        }               \
    } while (0)
#endif

#define DRV_ERR_OK         E_OK
#define DRV_ERR_EOK        E_OK
#define DRV_ERR_ERROR      E_ERROR
#define DRV_ERR_POINT_NONE E_POINT_NONE
#define DRV_ERR_WRONG_ARGS E_WRONG_ARGS
#define DRV_ERR_BUSY       E_BUSY
#define DRV_ERR_TIMEOUT    E_TIME_OUT
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
