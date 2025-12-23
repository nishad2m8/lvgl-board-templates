//pin assignments for Waveshare 1.8" amoled esp32-s3 module

#pragma once

/***********************config*************************/

//#define SPI_FREQUENCY         20000000 // at 40MHz we get artifacts when using software rotation
//#define TFT_SPI_MODE          SPI_MODE0
//#define TFT_SPI_HOST          SPI2_HOST


// Display
#define EXAMPLE_LCD_H_RES 720
#define EXAMPLE_LCD_V_RES 1280

// #define TFT_QSPI_CS           12
// #define TFT_QSPI_SCK          11
// #define TFT_QSPI_D0           4
// #define TFT_QSPI_D1           5
// #define TFT_QSPI_D2           6
// #define TFT_QSPI_D3           7
// #define TFT_QSPI_RST          -1 //done using port expander

#define LVGL_LCD_BUF_SIZE     (EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES)
#define SEND_BUF_SIZE         (EXAMPLE_LCD_H_RES * 10)





// ES8311
// #define I2S_MCK_IO 16
// #define I2S_BCK_IO 9
// #define I2S_DI_IO 10
// #define I2S_WS_IO 45
// #define I2S_DO_IO 8

// #define MCLKPIN             16
// #define BCLKPIN              9
// #define WSPIN               45
// #define DOPIN               10
// #define DIPIN                8
// #define PA                  46

// Battery Voltage ADC
//#define BATTERY_VOLTAGE_ADC_DATA -1

// SD
//#define SD_CS -1
//#define SD_MOSI -1
//#define SD_MISO -1
//#define SD_SCLK -1

// RTC
//#define PCF8563_INT -1

//IMU
//#define IMU_INT -1






