#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LVGL port configuration structure
 */
typedef struct {
    uint16_t display_width;
    uint16_t display_height;
    uint16_t tick_period_ms;
    uint16_t task_max_delay_ms;
    uint16_t task_min_delay_ms;
} lvgl_port_config_t;

/**
 * @brief Initialize and configure LVGL library
 *
 * @param config Configuration parameters for LVGL port
 *
 * This function initializes LVGL, creates display buffer, sets up tick timer,
 * and creates the LVGL task. Call this once during setup.
 */
void lvgl_port_init(const lvgl_port_config_t *config);

/**
 * @brief Lock LVGL mutex for thread-safe operations
 *
 * @param timeout_ms Timeout in milliseconds (-1 for infinite wait)
 * @return true if lock acquired successfully, false otherwise
 */
bool lvgl_port_lock(int timeout_ms);

/**
 * @brief Unlock LVGL mutex
 */
void lvgl_port_unlock(void);

#ifdef __cplusplus
}
#endif

#endif // LVGL_PORT_H
