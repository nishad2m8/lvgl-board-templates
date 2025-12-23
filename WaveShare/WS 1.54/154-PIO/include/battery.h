#ifndef BATTERY_H
#define BATTERY_H

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize battery monitoring
void battery_init(void);

// Get battery voltage in millivolts
uint32_t battery_get_voltage_mv(void);

// Get battery percentage (0-100)
uint8_t battery_get_percentage(void);

// Check if battery is charging
bool battery_is_charging(void);

// Check if battery power is on
bool battery_is_power_on(void);

// Turn battery power on/off
void battery_power_on(void);
void battery_power_off(void);

#ifdef __cplusplus
}
#endif

#endif // BATTERY_H
