#include "adc_bsp.h"

#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "adc_bsp";

static adc_oneshot_unit_handle_t s_adc_unit_handle = NULL;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
static adc_cali_handle_t s_cali_handle = NULL;
static bool s_cali_enabled = false;
#endif

static bool s_adc_initialized = false;

static void adc_bsp_enable_calibration(void)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };

    if (adc_cali_create_scheme_curve_fitting(&cali_config, &s_cali_handle) == ESP_OK) {
        s_cali_enabled = true;
    } else {
        ESP_LOGW(TAG, "ADC calibration unavailable, falling back to raw scaling");
        s_cali_enabled = false;
        s_cali_handle = NULL;
    }
#endif
}

void adc_bsp_init(void)
{
    if (s_adc_initialized) {
        return;
    }

    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &s_adc_unit_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_unit_handle, ADC_CHANNEL_3, &chan_config));

    adc_bsp_enable_calibration();
    s_adc_initialized = true;
}

void adc_get_value(float *value, int *raw)
{
    if (!s_adc_initialized) {
        if (value) {
            *value = 0.0f;
        }
        if (raw) {
            *raw = 0;
        }
        return;
    }

    int adc_raw = 0;
    esp_err_t err = adc_oneshot_read(s_adc_unit_handle, ADC_CHANNEL_3, &adc_raw);
    if (raw) {
        *raw = adc_raw;
    }

    if (value) {
        float voltage = 0.0f;
        if (err == ESP_OK) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
            if (s_cali_enabled && s_cali_handle != NULL) {
                int voltage_mv = 0;
                if (adc_cali_raw_to_voltage(s_cali_handle, adc_raw, &voltage_mv) == ESP_OK) {
                    voltage = (voltage_mv / 1000.0f);
                } else {
                    ESP_LOGW(TAG, "Failed to convert calibrated voltage, using raw estimate");
                    voltage = ((float)adc_raw / 4095.0f) * 3.3f;
                }
            } else
#endif
            {
                voltage = ((float)adc_raw / 4095.0f) * 3.3f;
            }
        } else {
            ESP_LOGW(TAG, "ADC read failed (%s)", esp_err_to_name(err));
        }

        // Hardware divides the battery voltage by 3 before the ADC measurement.
        const float divider_factor = 3.0f;
        *value = voltage * divider_factor;
    } else if (err != ESP_OK) {
        ESP_LOGW(TAG, "ADC read failed (%s)", esp_err_to_name(err));
    }
}
