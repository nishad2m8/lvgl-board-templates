/*
 * SPDX-FileCopyrightText: 2023-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 * @file  esp_panel_board_supported_conf.h
 * @brief Configuration file for supported ESP development boards
 *
 * This file contains configuration options for various supported development boards using ESP Panel.
 * Users can select their specific board by uncommenting the corresponding macro definition.
 */

#pragma once

/**
 * @brief Flag to enable supported board configuration (0/1)
 *
 * Set to `1` to enable supported board configuration, `0` to disable
 */
#define ESP_PANEL_BOARD_DEFAULT_USE_SUPPORTED       (1)

#if ESP_PANEL_BOARD_DEFAULT_USE_SUPPORTED
/**
 * @brief Board selection macros
 *
 * Uncomment one of the following macros to select a supported development board. Multiple macros enabled
 * simultaneously will trigger a compilation error.
 */

/*
 * VIEWE Supported Boards (https://viewedisplay.com/):
 *
 * VIEWE Model Number Format (Take `UEDX24320024E` as an example):
 *  +--------+----+----+----+----+--------+
 *  |  UEDX  | 24 | 32 | 00 | 24 | E-WB-A |
 *  +--------+----+----+----+----+--------+
 *              |    |         |
 *              |    |         +---- Display size: 2.4 inch
 *              |    +-------------- Vertical resolution: 320
 *              +------------------- Horizontal resolution: 240
 *  So UEDX24320024E means: 240x320 resolution & 2.4 inch display
 *
 * - BOARD_VIEWE_UEDX48480021_MD80E_V2 (UEDX48480021_MD80E_V2): https://viewedisplay.com/product/esp32-2-1-inch-480x480-round-tft-knob-display-rotary-encoder-arduino-lvgl/
 */
#define BOARD_VIEWE_UEDX48480021_MD80E_V2

#endif /* ESP_PANEL_BOARD_DEFAULT_USE_SUPPORTED */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// File Version ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Current file version: v1.0.0
 *
 * Changelog:
 *  v1.0.0: Initial version.
 */
#define ESP_PANEL_BOARD_SUPPORTED_FILE_VERSION_MAJOR   1
#define ESP_PANEL_BOARD_SUPPORTED_FILE_VERSION_MINOR   0
#define ESP_PANEL_BOARD_SUPPORTED_FILE_VERSION_PATCH   0