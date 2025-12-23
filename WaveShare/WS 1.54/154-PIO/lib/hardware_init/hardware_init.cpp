#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "hardware_init.h"
#include "driver/gpio.h"
#include "user_config.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "board_power_bsp.h"

static const char *TAG = "hardware";

// Global hardware instances
epaper_driver_display *driver = NULL;
board_power_bsp_t board_div(EPD_PWR_PIN, Audio_PWR_PIN, VBAT_PWR_PIN);

/**
 * @brief Initialize hardware peripherals
 *
 * This function initializes:
 * - Power control for ePaper and Audio
 * - ePaper display driver
 */
void hardware_init(void)
{
  ESP_LOGI(TAG, "Initializing hardware");

  // Turn on power for peripherals
  board_div.POWEER_EPD_ON();
  board_div.POWEER_Audio_ON();

  // Initialize ePaper display
  custom_lcd_spi_t driver_config = {};
  driver_config.cs = EPD_CS_PIN;
  driver_config.dc = EPD_DC_PIN;
  driver_config.rst = EPD_RST_PIN;
  driver_config.busy = EPD_BUSY_PIN;
  driver_config.mosi = EPD_MOSI_PIN;
  driver_config.scl = EPD_SCK_PIN;
  driver_config.spi_host = EPD_SPI_NUM;
  driver_config.buffer_len = 5000;

  ESP_LOGI(TAG, "Creating ePaper driver (%dx%d)", EPD_WIDTH, EPD_HEIGHT);
  driver = new epaper_driver_display(EPD_WIDTH, EPD_HEIGHT, driver_config);
  driver->EPD_Init();
  driver->EPD_Clear();
  driver->EPD_DisplayPartBaseImage();
  driver->EPD_Init_Partial();  // Partial refresh initialization

  ESP_LOGI(TAG, "Hardware initialization complete");
}
