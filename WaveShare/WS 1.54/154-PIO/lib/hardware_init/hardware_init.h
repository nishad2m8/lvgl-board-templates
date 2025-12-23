#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include "epaper_driver_bsp.h"

// Global hardware driver instance
extern epaper_driver_display *driver;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize hardware peripherals
 *
 * This function initializes:
 * - Power control for ePaper and Audio
 * - ePaper display driver and SPI communication
 */
void hardware_init(void);

#ifdef __cplusplus
}
#endif

#endif // HARDWARE_INIT_H
