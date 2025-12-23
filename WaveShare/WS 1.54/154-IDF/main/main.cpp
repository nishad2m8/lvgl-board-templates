#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "user_config.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "epaper_driver_bsp.h"
#include "board_power_bsp.h"
#include "lvgl_port.h"
#include "ui.h"

static const char *TAG = "main_1_54";
static epaper_driver_display *driver = NULL;
static board_power_bsp_t board_div(EPD_PWR_PIN, Audio_PWR_PIN, VBAT_PWR_PIN);

static void led_test_task(void *arg)
{
    gpio_config_t gpio_conf = {};
    gpio_conf.pin_bit_mask = 0x1ULL << 3;
    gpio_conf.mode = GPIO_MODE_OUTPUT;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.intr_type = GPIO_INTR_DISABLE;

    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));
    for(;;)
    {
        gpio_set_level((gpio_num_t)3, 0);
        vTaskDelay(pdMS_TO_TICKS(200));
        gpio_set_level((gpio_num_t)3, 1);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

extern "C" void app_main(void)
{
    // Initialize board power
    ESP_LOGI(TAG, "Initializing board power");
    board_div.POWEER_EPD_ON();
    board_div.POWEER_Audio_ON();

    // Initialize e-paper display
    ESP_LOGI(TAG, "Initializing e-paper display");
    custom_lcd_spi_t driver_config = {};
    driver_config.cs = EPD_CS_PIN;
    driver_config.dc = EPD_DC_PIN;
    driver_config.rst = EPD_RST_PIN;
    driver_config.busy = EPD_BUSY_PIN;
    driver_config.mosi = EPD_MOSI_PIN;
    driver_config.scl = EPD_SCK_PIN;
    driver_config.spi_host = EPD_SPI_NUM;
    driver_config.buffer_len = 5000;

    driver = new epaper_driver_display(EPD_WIDTH, EPD_HEIGHT, driver_config);
    driver->EPD_Init();
    driver->EPD_Clear();
    driver->EPD_DisplayPartBaseImage();
    driver->EPD_Init_Partial();

    // Initialize LVGL using port component
    lv_display_t *disp = lvgl_port_init(driver, EPD_WIDTH, EPD_HEIGHT);
    if (disp == NULL) {
        ESP_LOGE(TAG, "Failed to initialize LVGL port");
        return;
    }

    // Start LVGL task
    lvgl_port_start_task();

    // Create LED test task
    ESP_LOGI(TAG, "Creating LED test task");
    xTaskCreatePinnedToCore(led_test_task, "led_test", 4 * 1024, NULL, 4, NULL, 1);

    // Initialize UI
    if (lvgl_port_lock(-1))
    {
        ESP_LOGI(TAG, "Initializing UI");
        ui_init();
        lvgl_port_unlock();
    }

    ESP_LOGI(TAG, "Initialization complete");
}
