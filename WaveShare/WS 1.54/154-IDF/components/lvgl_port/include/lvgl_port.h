#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include "lvgl.h"
#include "epaper_driver_bsp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LVGL port with e-paper display driver
 *
 * @param display_driver Pointer to the e-paper display driver
 * @param width Display width in pixels
 * @param height Display height in pixels
 * @return lv_display_t* Pointer to the created LVGL display object
 */
lv_display_t* lvgl_port_init(epaper_driver_display *display_driver, int width, int height);

/**
 * @brief Lock LVGL mutex for thread-safe operations
 *
 * @param timeout_ms Timeout in milliseconds (-1 for infinite)
 * @return true if lock acquired successfully
 * @return false if lock acquisition failed
 */
bool lvgl_port_lock(int timeout_ms);

/**
 * @brief Unlock LVGL mutex
 */
void lvgl_port_unlock(void);

/**
 * @brief Start LVGL port task for handling LVGL timer
 */
void lvgl_port_start_task(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_PORT_H
