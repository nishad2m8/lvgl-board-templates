#include "lvgl_port.h"
#include <Arduino.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_heap_caps.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl.h"

// External driver reference (from hardware_init library)
#include "hardware_init.h"

static const char *TAG = "lvgl_port";
static SemaphoreHandle_t lvgl_mux = NULL;
static lvgl_port_config_t port_config;

// Forward declarations
static void lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p);
static void lvgl_tick_callback(void *arg);
static void lvgl_port_task(void *arg);

/**
 * @brief Flush callback for LVGL display
 */
static void lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
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

/**
 * @brief Timer callback to increase LVGL tick
 */
static void lvgl_tick_callback(void *arg)
{
  lv_tick_inc(port_config.tick_period_ms);
}

/**
 * @brief LVGL task handler
 */
static void lvgl_port_task(void *arg)
{
  uint32_t task_delay_ms = port_config.task_max_delay_ms;
  for(;;)
  {
    if (lvgl_port_lock(-1))
    {
      task_delay_ms = lv_timer_handler();
      lvgl_port_unlock();
    }
    if (task_delay_ms > port_config.task_max_delay_ms)
    {
      task_delay_ms = port_config.task_max_delay_ms;
    } else if (task_delay_ms < port_config.task_min_delay_ms)
    {
      task_delay_ms = port_config.task_min_delay_ms;
    }
    vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
  }
}

/**
 * @brief Initialize LVGL port
 */
void lvgl_port_init(const lvgl_port_config_t *config)
{
  if (config == NULL) {
    ESP_LOGE(TAG, "Invalid configuration");
    return;
  }

  // Store configuration
  port_config = *config;

  ESP_LOGI(TAG, "Initializing LVGL (Display: %dx%d)", config->display_width, config->display_height);
  lv_init();

  // Create display
  lv_display_t * disp = lv_display_create(config->display_width, config->display_height);
  lv_display_set_flush_cb(disp, lvgl_flush_cb);

  // Calculate buffer size (RGB565 = 2 bytes per pixel)
  const size_t buff_size = config->display_width * config->display_height * 2;

  // Allocate display buffer (try SPIRAM first, fallback to regular heap)
  ESP_LOGI(TAG, "Allocating LVGL buffer: %d bytes", buff_size);
  uint8_t *buffer_1 = NULL;
  buffer_1 = (uint8_t *)heap_caps_malloc(buff_size, MALLOC_CAP_SPIRAM);
  if (buffer_1 == NULL) {
    ESP_LOGW(TAG, "SPIRAM allocation failed, trying regular heap");
    buffer_1 = (uint8_t *)heap_caps_malloc(buff_size, MALLOC_CAP_8BIT);
  }

  if (buffer_1 != NULL) {
    ESP_LOGI(TAG, "LVGL buffer allocated successfully at %p", buffer_1);
  } else {
    ESP_LOGE(TAG, "LVGL buffer allocation failed! Requested size: %d bytes", buff_size);
  }
  assert(buffer_1);
  lv_display_set_buffers(disp, buffer_1, NULL, buff_size, LV_DISPLAY_RENDER_MODE_FULL);

  // Install LVGL tick timer
  ESP_LOGI(TAG, "Installing LVGL tick timer (%d ms)", config->tick_period_ms);
  esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &lvgl_tick_callback,
    .name = "lvgl_tick"
  };
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, config->tick_period_ms * 1000));

  // Create mutex for thread safety
  lvgl_mux = xSemaphoreCreateMutex();
  assert(lvgl_mux);

  // Create LVGL task
  ESP_LOGI(TAG, "Creating LVGL task");
  xTaskCreatePinnedToCore(lvgl_port_task, "LVGL", 8 * 1024, NULL, 4, NULL, 1);

  ESP_LOGI(TAG, "LVGL port initialization complete");
}

/**
 * @brief Lock LVGL mutex
 */
bool lvgl_port_lock(int timeout_ms)
{
  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;
}

/**
 * @brief Unlock LVGL mutex
 */
void lvgl_port_unlock(void)
{
  assert(lvgl_mux && "lvgl_port_init must be called first");
  xSemaphoreGive(lvgl_mux);
}
