/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : flash_map.h
 * @Author       : Lu Xianfan
 * @Date         : 2026-03-16 11:39:57
 * @LastEditors  : Lu Xianfan
 * @LastEditTime : 2026-03-16 11:39:57
 * @Brief        : armfly-v7 flash map
 */

#ifndef __FLASH_MAP_H__
#define __FLASH_MAP_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CONFIG_FLASH_BASE              (0x08000000UL)
#define CONFIG_FLASH_SIZE              (0x00200000UL)
#define CONFIG_FLASH_END               (CONFIG_FLASH_BASE + CONFIG_FLASH_SIZE)
#define CONFIG_FLASH_BLOCK_SIZE        (0x00020000UL)

#define CONFIG_NORFLASH_BASE           CONFIG_FLASH_BASE
#define CONFIG_NORFLASH_SIZE           CONFIG_FLASH_SIZE
#define CONFIG_NORFLASH_END            CONFIG_FLASH_END
#define CONFIG_NORFLASH_BLKSIZE        CONFIG_FLASH_BLOCK_SIZE

#define CONFIG_DTCM_BASE               (0x20000000UL)
#define CONFIG_DTCM_SIZE               (0x00020000UL)
#define CONFIG_AXI_SRAM_BASE           (0x24000000UL)
#define CONFIG_AXI_SRAM_SIZE           (0x00080000UL)
#define CONFIG_SRAM_D2_BASE            (0x30000000UL)
#define CONFIG_SRAM_D2_SIZE            (0x00048000UL)
#define CONFIG_SRAM_D3_BASE            (0x38000000UL)
#define CONFIG_SRAM_D3_SIZE            (0x00010000UL)

#define CONFIG_CHIP_UUID_BASE          (0x1FF1E800UL)
#define CONFIG_CHIP_UUID_SIZE          (12UL)
#define CONFIG_FLASH_SIZE_REGISTER     (0x1FF1E880UL)

#ifdef __cplusplus
}
#endif
#endif
