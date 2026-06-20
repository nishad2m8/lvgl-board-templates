/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "board_init.h"
#include <assert.h>
#include <bsp/m5stack_tab5.h>
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void board_init(void)
{
    ESP_ERROR_CHECK(bsp_i2c_init());

    i2c_master_bus_handle_t i2c_bus_handle = bsp_i2c_get_handle();
    bsp_io_expander_pi4ioe_init(i2c_bus_handle);

    bsp_set_charge_qc_en(true);
    vTaskDelay(pdMS_TO_TICKS(50));
    bsp_set_charge_en(true);

    bsp_reset_tp();

    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * BSP_LCD_V_RES,
        .double_buffer = true,
        .flags =
            {
#if CONFIG_BSP_LCD_COLOR_FORMAT_RGB888
                .buff_dma = false,
#else
                .buff_dma = true,
#endif
                .buff_spiram = true,
                .sw_rotate = true,
            },
    };

    lv_display_t *display = bsp_display_start_with_config(&cfg);
    assert(display != NULL);

    lv_display_set_rotation(display, LV_DISPLAY_ROTATION_90);
    ESP_ERROR_CHECK(bsp_display_backlight_on());
    bsp_display_unlock();
}

bool board_ui_lock(uint32_t timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void board_ui_unlock(void)
{
    lvgl_port_unlock();
}
