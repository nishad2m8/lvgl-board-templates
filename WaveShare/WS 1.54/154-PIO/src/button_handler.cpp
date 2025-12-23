#include "button_handler.h"
#include "button_bsp.h"
#include "multi_button.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "user_config.h"

static const char *TAG = "button_handler";
static button_event_callback_t user_callback = NULL;

extern "C" void button_handler_init(void)
{
    ESP_LOGI(TAG, "Initializing button handler");
    user_button_init();
    ESP_LOGI(TAG, "Button handler initialized");
}

extern "C" uint8_t button_handler_get_pwr_repeat_count(void)
{
    return user_button_get_repeat_count();
}

extern "C" uint8_t button_handler_get_boot_repeat_count(void)
{
    return user_boot_get_repeat_count();
}

extern "C" void button_handler_register_callback(button_event_callback_t callback)
{
    user_callback = callback;
}
