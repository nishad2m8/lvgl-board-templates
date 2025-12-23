#include "temp_humidity.h"
#include "i2c_equipment.h"
#include "i2c_bsp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "temp_humidity";
static i2c_equipment_shtc3 *shtc3_dev = NULL;

extern "C" void temp_humidity_init(void)
{
    ESP_LOGI(TAG, "Initializing SHTC3 sensor");

    // Initialize I2C master
    i2c_master_Init();

    // Create SHTC3 device instance
    shtc3_dev = new i2c_equipment_shtc3();

    ESP_LOGI(TAG, "SHTC3 sensor initialized, ID: 0x%04x", shtc3_dev->get_Shtc3Id());
}

extern "C" float temp_humidity_get_temperature(void)
{
    if (shtc3_dev == NULL) {
        ESP_LOGE(TAG, "SHTC3 not initialized");
        return 0.0f;
    }

    shtc3_data_t data = shtc3_dev->readTempHumi();
    return data.Temp;
}

extern "C" float temp_humidity_get_humidity(void)
{
    if (shtc3_dev == NULL) {
        ESP_LOGE(TAG, "SHTC3 not initialized");
        return 0.0f;
    }

    shtc3_data_t data = shtc3_dev->readTempHumi();
    return data.RH;
}

extern "C" void temp_humidity_read(float *temperature, float *humidity)
{
    if (shtc3_dev == NULL) {
        ESP_LOGE(TAG, "SHTC3 not initialized");
        if (temperature) *temperature = 0.0f;
        if (humidity) *humidity = 0.0f;
        return;
    }

    shtc3_data_t data = shtc3_dev->readTempHumi();
    if (temperature) *temperature = data.Temp;
    if (humidity) *humidity = data.RH;
}
