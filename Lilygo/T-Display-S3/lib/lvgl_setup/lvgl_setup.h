#ifndef LVGL_SETUP_H
#define LVGL_SETUP_H

#include <lvgl.h>
#include <TFT_eSPI.h>

// Screen resolution configuration
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 170

// Display rotation options
#define ROTATION_HOME_DOWN      0  // Portrait mode - 170x320
#define ROTATION_LANDSCAPE_CW   1  // Landscape mode - 320x170 (default)
#define ROTATION_HOME_UP        2  // Upside-down portrait
#define ROTATION_LANDSCAPE_CCW  3  // Counter-clockwise landscape

// Function declarations
void lvgl_setup_init(uint8_t rotation = ROTATION_LANDSCAPE_CW);
void lvgl_setup_tick_inc(void);

// External TFT instance
extern TFT_eSPI tft;

#endif // LVGL_SETUP_H