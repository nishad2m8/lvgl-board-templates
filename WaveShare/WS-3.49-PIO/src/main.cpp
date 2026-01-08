#include <Arduino.h>
#include "user_config.h"
#include "lvgl_port.h"
#include "esp_sleep.h"
#include "i2c_bsp.h"
#include "lcd_bl_pwm_bsp.h"
#include "button_bsp.h"
#include "adc_bsp.h"
#include "driver/gpio.h"
#include "rtc_bsp.h"

#include "ui.h"  // Include the UI layout generated with EEZ Studio

static bool device_powered_on = true;
static bool allow_shutdown = true;
static bool ignore_next_release = false;
static bool woke_from_deep_sleep = false;
static constexpr gpio_num_t POWER_BUTTON_GPIO = GPIO_NUM_16;

void setup()
{
  const esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();
  woke_from_deep_sleep = (wake_cause == ESP_SLEEP_WAKEUP_EXT1);
  device_powered_on = !woke_from_deep_sleep;
  allow_shutdown = !woke_from_deep_sleep;
  ignore_next_release = woke_from_deep_sleep;

  i2c_master_Init();
  Serial.begin(115200);
  rtc_bsp_init();

  if (woke_from_deep_sleep)
  {
    Serial.println("Woke from deep sleep via power button. Hold the button for ~2 s to power on.");
  }

  button_Init();
  adc_bsp_init(); 
  
  lvgl_port_init();
  lvgl_port_run_with_gui(ui_init); // Launch the LVGL GUI created in EEZ Studio
  lcd_bl_pwm_bsp_init(LCD_PWM_MODE_255);
  setUpduty(device_powered_on ? LCD_PWM_MODE_255 : LCD_PWM_MODE_0);
}

void loop()
{
  static uint32_t last_battery_log_ms = 0;
  const EventBits_t power_button_release_event = set_bit_button(2);
  const EventBits_t power_button_long_press_event = set_bit_button(1);
  const EventBits_t power_button_events = power_button_release_event | power_button_long_press_event;
  EventBits_t triggered_bits = xEventGroupWaitBits(
      pwr_groups,
      power_button_events,
      pdTRUE,
      pdFALSE,
      pdMS_TO_TICKS(50));

  if (triggered_bits & power_button_long_press_event)
  {
    if (!allow_shutdown)
    {
      Serial.println("Power button long press detected; powering up.");
      allow_shutdown = true;
      device_powered_on = true;
      setUpduty(LCD_PWM_MODE_255); // Bring backlight on after wake sequence
      ignore_next_release = true; // Skip the release that follows this wake long press
    }
    else
    {
      Serial.println("Power button long press detected; entering shutdown.");
      device_powered_on = false;
      setUpduty(LCD_PWM_MODE_0); // Ensure backlight is off before deep sleep
      esp_err_t wake_err = esp_sleep_enable_ext1_wakeup(1ULL << POWER_BUTTON_GPIO, ESP_EXT1_WAKEUP_ALL_LOW);
      if (wake_err != ESP_OK)
      {
        Serial.printf("Failed to configure wakeup source: %s\n", esp_err_to_name(wake_err));
        device_powered_on = true;
        setUpduty(LCD_PWM_MODE_255); // Re-enable backlight because shutdown is aborted
        ignore_next_release = true; // Consume the pending release
      }
      else
      {
        Serial.flush();
        esp_deep_sleep_start(); // Enter deep sleep; device will restart on wake
      }
    }
  }
  else if (triggered_bits & power_button_release_event)
  {
    if (ignore_next_release)
    {
      ignore_next_release = false;
    }
    else
    {
      device_powered_on = !device_powered_on;
      if (device_powered_on)
      {
        setUpduty(LCD_PWM_MODE_255); // Turn on the backlight
      }
      else
      {
        setUpduty(LCD_PWM_MODE_0); // Turn off the backlight
      }
    }
  }

  const uint32_t now = millis();
  if (now - last_battery_log_ms >= 1000UL)
  {
    float battery_voltage = 0.0f;
    adc_get_value(&battery_voltage, NULL);
    // Serial.printf("Battery voltage: %.2f V\n", battery_voltage); // Temporarily disabled
    if (rtc_bsp_is_ready())
    {
      const RtcDateTime_t rtc_time = i2c_rtc_get();
      // Serial.printf("RTC time: %04u/%02u/%02u %02u:%02u:%02u (Week %u)\n",
      //               rtc_time.year,
      //               rtc_time.month,
      //               rtc_time.day,
      //               rtc_time.hour,
      //               rtc_time.minute,
      //               rtc_time.second,
      //               rtc_time.week);
    }
    last_battery_log_ms = now;
  }
  vTaskDelay(pdMS_TO_TICKS(5));
}
