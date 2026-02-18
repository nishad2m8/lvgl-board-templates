#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

extern "C" {
#include "pcf85063_bsp.h"
}

struct SensorData {
    float temperature;
    float humidity;
    float acc[3];
    float gyro[3];
};

// Initialize I2C bus, AXP2101 PMIC (enables ALDO3 power rail), and all sensors.
// Must be called before epd_hardware_init() since it powers the EPD via ALDO3.
void sensors_init(void);

// Read SHTC3 temperature/humidity and QMI8658 accelerometer/gyroscope.
void sensors_read_all(SensorData *out);

// Read PCF85063 RTC.
Time_data sensors_get_time(void);

// Read battery percentage from AXP2101 (-1 if unavailable).
int sensors_get_battery_pct(void);

#endif // SENSORS_H
