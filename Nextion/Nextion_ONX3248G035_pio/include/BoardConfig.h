#pragma once

#include <Arduino.h>

// ONX3248G035 pins and addresses, taken from the factory examples.

static const uint8_t NEXTION_IIC_SCL_PIN = 7;
static const uint8_t NEXTION_IIC_SDA_PIN = 8;

static const uint16_t LCD_H_RES = 320;
static const uint16_t LCD_V_RES = 480;

static const int8_t LCD_SCLK_PIN = 5;
static const int8_t LCD_MOSI_PIN = 1;
static const int8_t LCD_MISO_PIN = -1;
static const uint8_t LCD_DC_PIN = 3;
static const uint8_t LCD_CS_PIN = 2;
static const uint8_t LCD_BL_PIN = 6;

static const uint8_t CST826_ADDR = 0x15;
static const uint8_t PCF8574_ADDR = 0x38;

// LCD reset hangs off the expander, not a GPIO.
static const uint8_t PCF8574_PIN_LCD_RST = 6;

// Don't touch: this orientation is the one that matches the glass.
#define DISPLAY_SWAP_XY   false
#define DISPLAY_MIRROR_X  true
#define DISPLAY_MIRROR_Y  false

static const uint32_t LCD_SPI_FREQ = 80000000;

// Lines per LVGL flush.
static const uint16_t LVGL_BUF_LINES = 40;
