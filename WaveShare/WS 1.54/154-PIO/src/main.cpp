#include <Arduino.h>
#include <esp_log.h>
#include "user_config.h"
#include "hardware_init.h"
#include "lvgl_port.h"
#include "ui.h"
#include "screens.h"
#include "battery.h"
#include "temp_humidity.h"
#include "button_handler.h"
#include "power/board_power_bsp.h"

static const char *TAG = "main";
extern board_power_bsp_t board_div;

// Task to update UI labels with sensor data
void ui_update_task(void *arg)
{
  char buffer[32];
  float temperature, humidity;

  for(;;)
  {
    // Read temperature and humidity
    temp_humidity_read(&temperature, &humidity);

    // Get battery percentage
    uint8_t battery_pct = battery_get_percentage();

    // Lock LVGL and update labels
    if (lvgl_port_lock(-1)) {
      // Update temperature label
      snprintf(buffer, sizeof(buffer), "%.1f°C", temperature);
      lv_label_set_text(objects.label_temp, buffer);

      // Update humidity label
      snprintf(buffer, sizeof(buffer), "%.1f%%", humidity);
      lv_label_set_text(objects.label_hum, buffer);

      // Update battery label
      snprintf(buffer, sizeof(buffer), "%d%%", battery_pct);
      lv_label_set_text(objects.label_battery, buffer);

      lvgl_port_unlock();
    }

    // Update every 2 seconds
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// Task to handle button events for battery power control
void button_power_task(void *arg)
{
  for(;;)
  {
    EventBits_t event = xEventGroupWaitBits(pwr_groups, set_bit_all, pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));

    // Handle long press (bit 2) - turn off battery
    if(get_bit_button(event, 2))
    {
      if(battery_is_power_on())
      {
        ESP_LOGI(TAG, "Button long press - turning battery OFF");
        if (lvgl_port_lock(-1)) {
          lv_label_set_text(objects.label_battery, "OFF");
          lvgl_port_unlock();
        }
        vTaskDelay(pdMS_TO_TICKS(200));
        board_div.VBAT_POWER_OFF();
        battery_power_off();
      }
    }
    // Handle press up (bit 3) - turn on battery
    else if(get_bit_button(event, 3))
    {
      if(!battery_is_power_on())
      {
        ESP_LOGI(TAG, "Button press up - turning battery ON");
        battery_power_on();
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  ESP_LOGI(TAG, "ESP32-S3 ePaper 1.54 Starting...");

  // Initialize hardware (power, epaper driver)
  hardware_init();

  // Initialize button handler
  button_handler_init();

  // Initialize battery monitoring
  battery_init();

  // Turn on VBAT power
  board_div.VBAT_POWER_ON();
  battery_power_on();

  // Initialize temperature and humidity sensor
  temp_humidity_init();

  // Configure LVGL port
  lvgl_port_config_t lvgl_config = {
    .display_width = EPD_WIDTH,
    .display_height = EPD_HEIGHT,
    .tick_period_ms = EXAMPLE_LVGL_TICK_PERIOD_MS,
    .task_max_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS,
    .task_min_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS
  };

  // Initialize LVGL
  lvgl_port_init(&lvgl_config);

  // Initialize UI (lock LVGL before creating UI elements)
  if (lvgl_port_lock(-1)) {
    ui_init();  // Custom UI initialization
    lvgl_port_unlock();
  }

  // Create UI update task
  xTaskCreatePinnedToCore(ui_update_task, "ui_update_task", 4 * 1024, NULL, 4, NULL, 1);

  // Create button power control task
  xTaskCreatePinnedToCore(button_power_task, "button_power_task", 4 * 1024, NULL, 4, NULL, 1);

  ESP_LOGI(TAG, "Setup complete");
}

void loop()
{
  // Main loop - LVGL runs in its own task
  vTaskDelay(pdMS_TO_TICKS(1000));
}
