#ifndef EPD_DISPLAY_H
#define EPD_DISPLAY_H

#include <lvgl.h>
#include <stdint.h>

#define EPD_WIDTH   800
#define EPD_HEIGHT  480

struct EpdRegion {
    int32_t x, y, w, h;
};

// Initialize EPD GPIO, SPI, and clear the display.
// Call after sensors_init() (which enables the ALDO3 power rail).
void epd_hardware_init(void);

// Initialize LVGL, allocate PSRAM framebuffers, create the lv_display.
// Call after epd_hardware_init().
void epd_lvgl_setup(void);

// Suppress/allow EPD refresh during LVGL flush.
// When suppressed, flush only updates the in-memory framebuffer.
void epd_set_suppress_refresh(bool suppress);

// Returns true until the first full base refresh completes.
bool epd_needs_full_refresh(void);

// Push a screen region to the e-paper via partial refresh.
// Coordinates are in LVGL pixel space (Y-inversion handled internally).
void epd_refresh_region(const EpdRegion &region);

#endif // EPD_DISPLAY_H
