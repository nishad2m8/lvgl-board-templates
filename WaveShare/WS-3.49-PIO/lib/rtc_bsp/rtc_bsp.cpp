#include "rtc_bsp.h"

#include <Arduino.h>
#include <limits.h>

#include "SensorPCF85063.hpp"
#include "i2c_bsp.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "rtc_bsp";

static SensorPCF85063 rtc;
static bool rtc_ready = false;

static bool rtc_i2c_transfer(uint8_t addr, uint8_t reg, uint8_t *buf, size_t len, bool writeReg, bool isWrite)
{
  (void)addr;
  if (!rtc_dev_handle)
  {
    ESP_LOGW(TAG, "RTC device handle not initialised yet.");
    return false;
  }

  if (len == 0)
  {
    return true;
  }

  if (len > UINT8_MAX)
  {
    ESP_LOGE(TAG, "Transfer length %u exceeds UINT8_MAX.", static_cast<unsigned>(len));
    return false;
  }

  const int reg_addr = writeReg ? static_cast<int>(reg) : -1;
  esp_err_t err = ESP_FAIL;

  if (isWrite)
  {
    err = static_cast<esp_err_t>(i2c_write_buff(rtc_dev_handle, reg_addr, buf, static_cast<uint8_t>(len)));
  }
  else
  {
    err = static_cast<esp_err_t>(i2c_read_buff(rtc_dev_handle, reg_addr, buf, static_cast<uint8_t>(len)));
  }

  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "I2C %s failed (reg=%d, len=%u, err=0x%x)",
             isWrite ? "write" : "read",
             reg_addr,
             static_cast<unsigned>(len),
             err);
    return false;
  }
  return true;
}

void rtc_bsp_init(void)
{
  if (rtc_ready)
  {
    return;
  }

  if (!rtc_dev_handle)
  {
    ESP_LOGE(TAG, "rtc_dev_handle is NULL. Call i2c_master_Init() before rtc_bsp_init().");
    return;
  }

  if (!rtc.begin(rtc_i2c_transfer))
  {
    ESP_LOGE(TAG, "Failed to initialise PCF85063 RTC.");
    return;
  }

  rtc_ready = true;
  ESP_LOGI(TAG, "PCF85063 RTC initialised.");
}

bool rtc_bsp_is_ready(void)
{
  return rtc_ready;
}

void i2c_rtc_setTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
  if (!rtc_ready)
  {
    ESP_LOGW(TAG, "RTC not ready. Ignoring setTime request.");
    return;
  }
  rtc.setDateTime(year, month, day, hour, minute, second);
}

RtcDateTime_t i2c_rtc_get(void)
{
  RtcDateTime_t time = {};
  if (!rtc_ready)
  {
    ESP_LOGW(TAG, "RTC not ready. Returning zeroed timestamp.");
    return time;
  }

  const RTC_DateTime datetime = rtc.getDateTime();
  time.year = datetime.getYear();
  time.month = datetime.getMonth();
  time.day = datetime.getDay();
  time.hour = datetime.getHour();
  time.minute = datetime.getMinute();
  time.second = datetime.getSecond();
  time.week = datetime.getWeek();
  return time;
}

void i2c_rtc_loop_task(void *arg)
{
  (void)arg;
  for (;;)
  {
    if (!rtc_ready)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    const RTC_DateTime datetime = rtc.getDateTime();
    Serial.printf("%04u/%02u/%02u %02u:%02u:%02u (Week %u)\n",
                  datetime.getYear(),
                  datetime.getMonth(),
                  datetime.getDay(),
                  datetime.getHour(),
                  datetime.getMinute(),
                  datetime.getSecond(),
                  datetime.getWeek());
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
