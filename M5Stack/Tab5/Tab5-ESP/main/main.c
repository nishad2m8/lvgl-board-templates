/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "board_init.h"
#include <bsp/m5stack_tab5.h>
#include <esp_lcd_touch.h>
#include <esp_log.h>
#include "ui/ui.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "main";

static void log_touch_state(void)
{
    static bool last_pressed = false;
    static uint16_t last_x = UINT16_MAX;
    static uint16_t last_y = UINT16_MAX;

    esp_lcd_touch_handle_t touch = bsp_display_get_touch_handle();
    if (touch == NULL) {
        return;
    }

    uint16_t touch_x[1] = {0};
    uint16_t touch_y[1] = {0};
    uint8_t touch_cnt = 0;

    esp_lcd_touch_read_data(touch);
    bool pressed = esp_lcd_touch_get_coordinates(touch, touch_x, touch_y, NULL, &touch_cnt, 1);

    if (pressed && touch_cnt > 0 &&
        (!last_pressed || touch_x[0] != last_x || touch_y[0] != last_y)) {
        ESP_LOGI(TAG, "raw touch x=%u y=%u", touch_x[0], touch_y[0]);
    }

    last_pressed = pressed && touch_cnt > 0;
    last_x = last_pressed ? touch_x[0] : UINT16_MAX;
    last_y = last_pressed ? touch_y[0] : UINT16_MAX;
}

void app_main(void)
{
    board_init();

    if (board_ui_lock(0)) {
        ui_init();
        board_ui_unlock();
    }

    while (1) {
        if (board_ui_lock(0)) {
            ui_tick();
            log_touch_state();
            board_ui_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}
