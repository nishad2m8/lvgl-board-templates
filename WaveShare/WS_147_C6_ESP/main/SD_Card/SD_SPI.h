
#pragma once

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_flash.h"

#include "ST7789.h"

#define PIN_NUM_MOSI    EXAMPLE_PIN_NUM_MOSI    
#define PIN_NUM_MISO    5    
#define PIN_NUM_SCLK    EXAMPLE_PIN_NUM_SCLK    
#define PIN_NUM_CS      4            

esp_err_t SD_Card_CS_EN(void);
esp_err_t SD_Card_CS_Dis(void);

esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path);

extern uint32_t SDCard_Size;
extern uint32_t Flash_Size;
void SD_Init(void);
void Flash_Searching(void);