#ifndef __BSP_BATTERY_H__
#define __BSP_BATTERY_H__
#include <stdio.h>

#define EXAMPLE_BATTERY_ADC_CHANNEL ADC_CHANNEL_1
#define EXAMPLE_ADC_ATTEN ADC_ATTEN_DB_12
#define EXAMPLE_ADC_UNIT ADC_UNIT_2

#ifdef __cplusplus
extern "C" {
#endif

void bsp_battery_init(void);
void bsp_battery_get_voltage(float *voltage, uint16_t *adc_value);

#ifdef __cplusplus
}
#endif


#endif