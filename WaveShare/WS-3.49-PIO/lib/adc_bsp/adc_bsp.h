#ifndef ADC_BSP_H
#define ADC_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the ADC peripheral for battery monitoring.
 */
void adc_bsp_init(void);

/**
 * @brief Read the battery voltage.
 *
 * @param value Pointer to store the computed voltage value in volts.
 * @param raw Pointer to store the raw ADC reading (optional, can be NULL).
 */
void adc_get_value(float *value, int *raw);

#ifdef __cplusplus
}
#endif

#endif /* ADC_BSP_H */
