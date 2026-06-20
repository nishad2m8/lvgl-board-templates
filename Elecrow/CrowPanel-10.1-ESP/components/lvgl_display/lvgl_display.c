#include "lvgl_display.h"

#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

#include "bsp_illuminate.h"
#include "bsp_display.h"

#define LVGL_DISPLAY_TAG "LVGL_DISPLAY"

extern esp_lcd_panel_handle_t panel_handle;
extern esp_lcd_panel_io_handle_t mipi_dbi_io;

esp_err_t lvgl_display_init(void)
{
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = configMAX_PRIORITIES - 4,
        .task_stack = 8192 * 2,
        .task_affinity = -1,
        .task_max_sleep_ms = 10,
        .timer_period_ms = 5,
    };
    esp_err_t err = lvgl_port_init(&lvgl_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(LVGL_DISPLAY_TAG, "lvgl_port_init failed: %s", esp_err_to_name(err));
        return err;
    }

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = mipi_dbi_io,
        .panel_handle = panel_handle,
        .control_handle = panel_handle,
        .buffer_size = (H_size * V_size * ((BITS_PER_PIXEL + 7) / 8)),
        .double_buffer = true,
        .hres = H_size,
        .vres = V_size,
        .monochrome = false,
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = false,
            .buff_spiram = true,
            .sw_rotate = true,
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = false,
#endif
#if CONFIG_DISPLAY_LVGL_FULL_REFRESH
            .full_refresh = true,
#else
            .full_refresh = false,
#endif
#if CONFIG_DISPLAY_LVGL_DIRECT_MODE
            .direct_mode = true,
#else
            .direct_mode = false,
#endif
        },
    };
    const lvgl_port_display_dsi_cfg_t lvgl_dpi_cfg = {
        .flags = {
#if CONFIG_DISPLAY_LVGL_AVOID_TEAR
            .avoid_tearing = true,
#else
            .avoid_tearing = false,
#endif
        },
    };
    lv_display_t *disp = lvgl_port_add_disp_dsi(&disp_cfg, &lvgl_dpi_cfg);
    if (disp == NULL) {
        ESP_LOGE(LVGL_DISPLAY_TAG, "lvgl_port_add_disp_dsi failed");
        return ESP_FAIL;
    }

    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = disp,
        .handle = tp,
    };
    lv_indev_t *indev = lvgl_port_add_touch(&touch_cfg);
    if (indev == NULL) {
        ESP_LOGE(LVGL_DISPLAY_TAG, "lvgl_port_add_touch failed");
        return ESP_FAIL;
    }
    return ESP_OK;
}
