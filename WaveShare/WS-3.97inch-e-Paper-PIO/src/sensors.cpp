#include "sensors.h"
#include <Arduino.h>

extern "C" {
#include "i2c_bsp.h"
#include "axp_prot.h"
#include "shtc3_bsp.h"
#include "qmi8658_bsp.h"
}

void sensors_init(void)
{
    Serial.println("[HW] Initializing I2C bus..."); Serial.flush();
    i2c_master_init();
    Serial.println("[HW] I2C bus OK"); Serial.flush();
    i2c_devices_init();
    Serial.println("[HW] I2C devices OK"); Serial.flush();

    Serial.println("[HW] Initializing AXP2101 power management..."); Serial.flush();
    axp_init();
    Serial.println("[HW] AXP2101 OK"); Serial.flush();
    Serial.println("[HW] Enabling e-Paper power rail (ALDO3)..."); Serial.flush();
    enapwrstate(ALDO3);
    delay(100);  // Allow power rail to stabilize

    Serial.println("[HW] Initializing SHTC3..."); Serial.flush();
    i2c_shtc3_init();
    Serial.println("[HW] Initializing QMI8658..."); Serial.flush();
    QMI8658_init();
    Serial.println("[HW] Initializing PCF85063..."); Serial.flush();
    PCF85063_init();
}

void sensors_read_all(SensorData *out)
{
    SHTC3_GetEnvTemperatureHumidity(&out->temperature, &out->humidity);

    unsigned int timestamp;
    QMI8658_read_xyz(out->acc, out->gyro, &timestamp);
}

Time_data sensors_get_time(void)
{
    return PCF85063_GetTime();
}

int sensors_get_battery_pct(void)
{
    return get_battery_power();
}
