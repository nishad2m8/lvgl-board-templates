#include <stdio.h>
#include <string.h>
#include "i2c_bsp.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

static const char *TAG = "I2C_BSP";

#if I2C_BSP_USE_LEGACY_DRIVER

#include <stdlib.h>

void i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = ESP32_SDA_NUM,
        .scl_io_num = ESP32_SCL_NUM,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
        .clk_flags = 0,
    };

    esp_err_t ret = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
        return;
    }

    ret = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "I2C driver already installed, reusing existing instance");
    } else if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Legacy I2C master initialized on port %d", I2C_MASTER_NUM);
}

esp_err_t i2c_devices_init(void)
{
    // No per-device handles are created when using the legacy driver.
    ESP_LOGI(TAG, "Legacy I2C driver in use, all devices share the same bus");
    return ESP_OK;
}

static esp_err_t i2c_write_with_reg(uint8_t dev_addr, uint8_t reg, const uint8_t *data, size_t len)
{
    size_t payload_len = len + 1;
    uint8_t *payload = (uint8_t *)malloc(payload_len);
    if (!payload) {
        ESP_LOGE(TAG, "Failed to allocate I2C TX buffer (%u bytes)", (unsigned)payload_len);
        return ESP_ERR_NO_MEM;
    }

    payload[0] = reg;
    if (len && data) {
        memcpy(payload + 1, data, len);
    }

    esp_err_t ret = i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, payload, payload_len, pdMS_TO_TICKS(1000));
    free(payload);
    return ret;
}

esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_write_with_reg(dev_addr, reg, data, len);
}

esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, dev_addr, &reg, 1, data, len, pdMS_TO_TICKS(1000));
}

esp_err_t i2c_write_bytes(uint8_t dev_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_to_device(I2C_MASTER_NUM, dev_addr, data, len, pdMS_TO_TICKS(1000));
}

esp_err_t i2c_read_bytes(uint8_t dev_addr, uint8_t *data, size_t len)
{
    return i2c_master_read_from_device(I2C_MASTER_NUM, dev_addr, data, len, pdMS_TO_TICKS(1000));
}

esp_err_t i2c_master_write_read_device_compat(uint8_t addr, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf, size_t rlen)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, addr, wbuf, wlen, rbuf, rlen, pdMS_TO_TICKS(1000));
}

#else

// I2C Master Bus Handle
i2c_master_bus_handle_t i2c_bus_handle = NULL;

// The handles of each device
i2c_master_dev_handle_t rtc_dev_handle = NULL;
i2c_master_dev_handle_t shtc3_dev_handle = NULL;
i2c_master_dev_handle_t axp2101_dev_handle = NULL;
i2c_master_dev_handle_t qmi8658_dev_handle = NULL;

void i2c_master_init(void)
{
    // Configure the I2C Master bus
    i2c_master_bus_config_t bus_config;
    memset(&bus_config, 0, sizeof(bus_config));
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.i2c_port = I2C_MASTER_NUM;
    bus_config.scl_io_num = ESP32_SCL_NUM;
    bus_config.sda_io_num = ESP32_SDA_NUM;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;

    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C master bus init failed: %s", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "I2C master bus initialized on port %d", I2C_MASTER_NUM);
}

// Add the device to the bus
esp_err_t i2c_bus_add_device(uint8_t dev_addr, uint32_t scl_speed_hz, i2c_master_dev_handle_t *dev_handle)
{
    if (!i2c_bus_handle) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = scl_speed_hz,
    };
    
    return i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, dev_handle);
}

// Initialize all I2C devices
esp_err_t i2c_devices_init(void)
{
    esp_err_t ret;
    
    // RTC (PCF85063)
    ret = i2c_bus_add_device(PCF85063Addr, I2C_MASTER_FREQ_HZ, &rtc_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to add RTC device: %s", esp_err_to_name(ret));
    }
    
    // Temperature and humidity sensor (SHTC3)
    ret = i2c_bus_add_device(SHTC3Addr, I2C_MASTER_FREQ_HZ, &shtc3_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to add SHTC3 device: %s", esp_err_to_name(ret));
    }
    
    // Power Management (AXP2101)
    ret = i2c_bus_add_device(AXP2101Addr, I2C_MASTER_FREQ_HZ, &axp2101_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to add AXP2101 device: %s", esp_err_to_name(ret));
    }
    
    // Six-axis sensor (QMI8658)
    ret = i2c_bus_add_device(QMI8658_SLAVE_ADDR_L, I2C_MASTER_FREQ_HZ, &qmi8658_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to add QMI8658 device: %s", esp_err_to_name(ret));
    }
    
    ESP_LOGI(TAG, "All I2C devices initialized");
    return ESP_OK;
}

// Obtain the corresponding handle based on the device address
static i2c_master_dev_handle_t get_dev_handle(uint8_t dev_addr)
{
    if (dev_addr == PCF85063Addr && rtc_dev_handle) return rtc_dev_handle;
    if (dev_addr == SHTC3Addr && shtc3_dev_handle) return shtc3_dev_handle;
    if (dev_addr == AXP2101Addr && axp2101_dev_handle) return axp2101_dev_handle;
    if (dev_addr == QMI8658_SLAVE_ADDR_L && qmi8658_dev_handle) return qmi8658_dev_handle;
    return NULL;
}

// Write to the register (with register address）
esp_err_t i2c_write_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_master_dev_handle_t dev_handle = get_dev_handle(dev_addr);
    
    // If there is no pre-created handle, create a temporary one
    if (!dev_handle) {
        esp_err_t ret = i2c_bus_add_device(dev_addr, I2C_MASTER_FREQ_HZ, &dev_handle);
        if (ret != ESP_OK) return ret;
    }
    
    uint8_t buf[len + 1];
    buf[0] = reg;
    memcpy(&buf[1], data, len);
    
    esp_err_t ret = i2c_master_transmit(dev_handle, buf, len + 1, pdMS_TO_TICKS(1000));
    
    // If the handle is created temporarily, it needs to be deleted
    if (get_dev_handle(dev_addr) == NULL) {
        i2c_master_bus_rm_device(dev_handle);
    }
    
    return ret;
}

// Read the register (write the register address first and then read the data)
esp_err_t i2c_read_reg(uint8_t dev_addr, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_master_dev_handle_t dev_handle = get_dev_handle(dev_addr);
    
    // If there is no pre-created handle, create a temporary one
    if (!dev_handle) {
        esp_err_t ret = i2c_bus_add_device(dev_addr, I2C_MASTER_FREQ_HZ, &dev_handle);
        if (ret != ESP_OK) return ret;
    }
    
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, &reg, 1, data, len, pdMS_TO_TICKS(1000));
    
    // If the handle is created temporarily, it needs to be deleted
    if (get_dev_handle(dev_addr) == NULL) {
        i2c_master_bus_rm_device(dev_handle);
    }
    
    return ret;
}

// Only write data (without register addresses)
esp_err_t i2c_write_bytes(uint8_t dev_addr, uint8_t *data, size_t len)
{
    i2c_master_dev_handle_t dev_handle = get_dev_handle(dev_addr);
    
    // If there is no pre-created handle, create a temporary one
    if (!dev_handle) {
        esp_err_t ret = i2c_bus_add_device(dev_addr, I2C_MASTER_FREQ_HZ, &dev_handle);
        if (ret != ESP_OK) return ret;
    }
    
    esp_err_t ret = i2c_master_transmit(dev_handle, data, len, pdMS_TO_TICKS(1000));
    
    // If the handle is created temporarily, it needs to be deleted
    if (get_dev_handle(dev_addr) == NULL) {
        i2c_master_bus_rm_device(dev_handle);
    }
    
    return ret;
}

// Read-only data (without register address)
esp_err_t i2c_read_bytes(uint8_t dev_addr, uint8_t *data, size_t len)
{
    i2c_master_dev_handle_t dev_handle = get_dev_handle(dev_addr);
    
    // If there is no pre-created handle, create a temporary one
    if (!dev_handle) {
        esp_err_t ret = i2c_bus_add_device(dev_addr, I2C_MASTER_FREQ_HZ, &dev_handle);
        if (ret != ESP_OK) return ret;
    }
    
    esp_err_t ret = i2c_master_receive(dev_handle, data, len, pdMS_TO_TICKS(1000));
    
    // If the handle is created temporarily, it needs to be deleted
    if (get_dev_handle(dev_addr) == NULL) {
        i2c_master_bus_rm_device(dev_handle);
    }
    
    return ret;
}

// Functions compatible with old code
esp_err_t i2c_master_write_read_device_compat(uint8_t addr, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf, size_t rlen)
{
    i2c_master_dev_handle_t dev_handle = get_dev_handle(addr);
    
    // If there is no pre-created handle, create a temporary one
    if (!dev_handle) {
        esp_err_t ret = i2c_bus_add_device(addr, I2C_MASTER_FREQ_HZ, &dev_handle);
        if (ret != ESP_OK) return ret;
    }
    
    esp_err_t ret = i2c_master_transmit_receive(dev_handle, wbuf, wlen, rbuf, rlen, pdMS_TO_TICKS(1000));
    
    // If the handle is created temporarily, it needs to be deleted
    if (get_dev_handle(addr) == NULL) {
        i2c_master_bus_rm_device(dev_handle);
    }
    
    return ret;
}

#endif // I2C_BSP_USE_LEGACY_DRIVER
