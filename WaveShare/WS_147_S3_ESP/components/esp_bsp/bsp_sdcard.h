#ifndef __BSP_SDCARD_H_
#define __BSP_SDCARD_H_
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include <stdio.h>

#define EXAMPLE_PIN_SD_CLK GPIO_NUM_16
#define EXAMPLE_PIN_SD_CMD GPIO_NUM_15
#define EXAMPLE_PIN_SD_D0 GPIO_NUM_17
#define EXAMPLE_PIN_SD_D1 GPIO_NUM_18
#define EXAMPLE_PIN_SD_D2 GPIO_NUM_13
#define EXAMPLE_PIN_SD_D3 GPIO_NUM_14


#ifdef __cplusplus
extern "C" {
#endif

void bsp_sdcard_init(void);
uint64_t bsp_sdcard_get_size(void);

#ifdef __cplusplus
}
#endif


#endif
