#pragma once

#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "Arduino_DriveBus_Library.h"
#include <Adafruit_XCA9554.h>
#include "pin_config.h"

// External references for display and touch
extern Arduino_SH8601 *gfx;
extern std::unique_ptr<Arduino_IIC> FT3168;
extern Adafruit_XCA9554 expander;
extern uint32_t screenWidth;
extern uint32_t screenHeight;

// Initialize display hardware and LVGL
void display_init();

// Initialize touch input
void touch_init();

// LVGL tick timer callback
void lvgl_tick_callback(void *arg);
