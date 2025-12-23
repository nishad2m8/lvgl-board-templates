#include "lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "user_config.h"

static const char *TAG = "lvgl_port";
static SemaphoreHandle_t lvgl_mux = NULL;
static epaper_driver_display *driver = NULL;

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))

/* LVGL flush callback */
static void lvgl_port_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    uint16_t *buffer = (uint16_t *)color_p;
    driver->EPD_Clear();
    for(int y = area->y1; y <= area->y2; y++)
    {
        for(int x = area->x1; x <= area->x2; x++)
        {
            uint8_t color = (*buffer < 0x7fff) ? DRIVER_COLOR_BLACK : DRIVER_COLOR_WHITE;
            driver->EPD_DrawColorPixel(x, y, color);
            buffer++;
        }
    }
    driver->EPD_DisplayPart();
    lv_disp_flush_ready(disp);
}

/* LVGL tick increase callback */
static void lvgl_port_increase_tick(void *arg)
{
    lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

/* LVGL port task */
static void lvgl_port_task(void *arg)
{
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    for(;;)
    {
        if (lvgl_port_lock(-1))
        {
            task_delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS)
        {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS)
        {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

lv_display_t* lvgl_port_init(epaper_driver_display *display_driver, int width, int height)
{
    if (display_driver == NULL) {
        ESP_LOGE(TAG, "Display driver is NULL");
        return NULL;
    }

    driver = display_driver;

    // Initialize LVGL
    ESP_LOGI(TAG, "Initializing LVGL");
    lv_init();

    // Create display
    lv_display_t * disp = lv_display_create(width, height);
    lv_display_set_flush_cb(disp, lvgl_port_flush_cb);

    // Allocate display buffer
    size_t buff_size = width * height * BYTES_PER_PIXEL;
    uint8_t *buffer_1 = (uint8_t *)heap_caps_malloc(buff_size, MALLOC_CAP_SPIRAM);
    if (buffer_1 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate display buffer");
        return NULL;
    }
    lv_display_set_buffers(disp, buffer_1, NULL, buff_size, LV_DISPLAY_RENDER_MODE_FULL);

    // Install LVGL tick timer
    ESP_LOGI(TAG, "Installing LVGL tick timer");
    esp_timer_create_args_t lvgl_tick_timer_args = {};
    lvgl_tick_timer_args.callback = &lvgl_port_increase_tick;
    lvgl_tick_timer_args.name = "lvgl_tick";
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_err_t ret = esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LVGL tick timer");
        return NULL;
    }
    ret = esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start LVGL tick timer");
        return NULL;
    }

    // Create mutex
    lvgl_mux = xSemaphoreCreateMutex();
    if (lvgl_mux == NULL) {
        ESP_LOGE(TAG, "Failed to create LVGL mutex");
        return NULL;
    }

    ESP_LOGI(TAG, "LVGL port initialized successfully");
    return disp;
}

bool lvgl_port_lock(int timeout_ms)
{
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

void lvgl_port_unlock(void)
{
    if (lvgl_mux == NULL) {
        ESP_LOGE(TAG, "LVGL mutex is NULL, lvgl_port_init must be called first");
        return;
    }
    xSemaphoreGive(lvgl_mux);
}

void lvgl_port_start_task(void)
{
    ESP_LOGI(TAG, "Starting LVGL port task");
    xTaskCreatePinnedToCore(lvgl_port_task, "LVGL", 8 * 1024, NULL, 4, NULL, 1);
}
