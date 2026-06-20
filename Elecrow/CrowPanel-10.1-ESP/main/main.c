// main.c
#include "main.h"
#include "esp_lvgl_port.h"
#include "ui.h"

/* LDO channel handles (power rails required by MIPI DSI PHY and panel) */
static esp_ldo_channel_handle_t ldo3 = NULL;
static esp_ldo_channel_handle_t ldo4 = NULL;

static void init_fail_handler(const char *module_name, esp_err_t err)
{
    while (1) {
        MAIN_ERROR("[%s] init failed: %s", module_name, esp_err_to_name(err));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void system_init(void)
{
    esp_err_t err;

    // 1. LDO rails for the screen
    esp_ldo_channel_config_t ldo3_cof = { .chan_id = 3, .voltage_mv = 2500 };
    err = esp_ldo_acquire_channel(&ldo3_cof, &ldo3);
    if (err != ESP_OK) init_fail_handler("ldo3", err);

    esp_ldo_channel_config_t ldo4_cof = { .chan_id = 4, .voltage_mv = 3300 };
    err = esp_ldo_acquire_channel(&ldo4_cof, &ldo4);
    if (err != ESP_OK) init_fail_handler("ldo4", err);
    MAIN_INFO("LDO3 and LDO4 init success");

    // 2. I2C bus for touch controller
    err = i2c_init();
    if (err != ESP_OK) init_fail_handler("I2C", err);
    MAIN_INFO("I2C init success");

    // 3. Touch panel
    err = touch_init();
    if (err != ESP_OK) init_fail_handler("Touch", err);
    MAIN_INFO("Touch panel init success");

    // 4. LCD hardware (DSI bus, panel, backlight off)
    err = display_init();
    if (err != ESP_OK) init_fail_handler("LCD", err);
    MAIN_INFO("LCD init success");

    // 5. LVGL port + display + touch indev
    err = lvgl_display_init();
    if (err != ESP_OK) init_fail_handler("LVGL display", err);
    MAIN_INFO("LVGL display init success");

    // 6. Backlight on
    err = set_lcd_blight(100);
    if (err != ESP_OK) init_fail_handler("LCD Backlight", err);
    MAIN_INFO("LCD backlight opened (brightness: 100)");

    // 7. LED GPIO
    err = gpio_extra_init();
    if (err != ESP_OK) init_fail_handler("GPIO48", err);
    gpio_extra_set_level(false);
    MAIN_INFO("LED initialized to OFF state");

    // 8. EEZ-generated UI (must hold LVGL mutex when called outside LVGL task)
    lvgl_port_lock(0);
    ui_init();
    lvgl_port_unlock();
    MAIN_INFO("UI created successfully");
}

void app_main(void)
{
    MAIN_INFO("Starting application...");
    system_init();
    MAIN_INFO("System initialized successfully");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
