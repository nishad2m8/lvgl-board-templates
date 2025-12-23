#ifndef TEMP_HUMIDITY_H
#define TEMP_HUMIDITY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize temperature and humidity sensor
void temp_humidity_init(void);

// Read temperature in Celsius
float temp_humidity_get_temperature(void);

// Read humidity in percentage
float temp_humidity_get_humidity(void);

// Read both temperature and humidity
void temp_humidity_read(float *temperature, float *humidity);

#ifdef __cplusplus
}
#endif

#endif // TEMP_HUMIDITY_H
