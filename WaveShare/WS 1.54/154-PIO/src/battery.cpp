#include "battery.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "user_config.h"
#include "power/board_power_bsp.h"

static const char *TAG = "battery";
static bool is_battery_on = false;

// ADC handles
static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_cali_handle_t cali_handle = NULL;
static bool adc_initialized = false;

extern "C" void battery_init(void)
{
    ESP_LOGI(TAG, "Initializing battery monitoring");

    // Check if battery power is already on (PWR_BUTTON_PIN state)
    gpio_config_t gpio_conf = {};
    gpio_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_conf.mode = GPIO_MODE_INPUT;
    gpio_conf.pin_bit_mask = (1ULL << PWR_BUTTON_PIN);
    gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));

    // Check initial battery power state
    is_battery_on = gpio_get_level(PWR_BUTTON_PIN);

    // Initialize ADC for battery voltage reading
    // Configure ADC calibration
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "ADC calibration failed, using raw values");
    }

    // Initialize ADC unit
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Configure ADC channel (ADC_CHANNEL_3 = GPIO4 for battery voltage)
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,    // 0-3.3V range
        .bitwidth = ADC_BITWIDTH_12, // 12-bit resolution (0-4095)
    };
    ESP_ERROR_CHECK_WITHOUT_ABORT(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));

    adc_initialized = true;

    ESP_LOGI(TAG, "Battery monitoring initialized, power state: %s", is_battery_on ? "ON" : "OFF");
}

extern "C" uint32_t battery_get_voltage_mv(void)
{
    if (!adc_initialized || adc1_handle == NULL) {
        ESP_LOGW(TAG, "ADC not initialized");
        return 3700; // Return default value
    }

    int adc_raw = 0;
    esp_err_t err = adc_oneshot_read(adc1_handle, ADC_CHANNEL_3, &adc_raw);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ADC read failed");
        return 3700; // Return default on error
    }

    int voltage_mv = 0;

    if (cali_handle != NULL) {
        // Use calibrated reading
        adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage_mv);
        // Hardware has voltage divider (divide by 2), so multiply by 2
        voltage_mv = voltage_mv * 2;
    } else {
        // Use raw calculation if calibration failed
        // voltage = (adc_raw * 3.3V / 4096) * 2 (for voltage divider)
        voltage_mv = (adc_raw * 3300 / 4096) * 2;
    }

    return (uint32_t)voltage_mv;
}

extern "C" uint8_t battery_get_percentage(void)
{
    uint32_t voltage_mv = battery_get_voltage_mv();

    // Simple linear mapping (3.0V = 0%, 4.2V = 100%)
    // This is a simplified model, real battery curve is non-linear
    const uint32_t min_voltage = 3000;
    const uint32_t max_voltage = 4200;

    if (voltage_mv <= min_voltage) return 0;
    if (voltage_mv >= max_voltage) return 100;

    uint8_t percentage = ((voltage_mv - min_voltage) * 100) / (max_voltage - min_voltage);
    return percentage;
}

extern "C" bool battery_is_charging(void)
{
    // TODO: Implement charging detection if hardware supports it
    return false;
}

extern "C" bool battery_is_power_on(void)
{
    return is_battery_on;
}

extern "C" void battery_power_on(void)
{
    is_battery_on = true;
    ESP_LOGI(TAG, "Battery power ON");
}

extern "C" void battery_power_off(void)
{
    is_battery_on = false;
    ESP_LOGI(TAG, "Battery power OFF");
}
