#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>

// Display resolution for 1.28 inch GC9A01
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// Initialize display hardware and LVGL
void display_init();

// Call AFTER ui_init() to start backlight
void display_start();

// Call in loop() to handle LVGL tasks
void display_update();

// Set backlight brightness (0-255)
void display_set_brightness(uint8_t brightness);

#endif // DISPLAY_H
