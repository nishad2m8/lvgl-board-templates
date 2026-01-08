#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>

// Display resolution
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 480

// Initialize display hardware and LVGL
void display_init();

// Call AFTER ui_init() to start backlight (factory sequence)
void display_start();

// Call in loop() to handle LVGL tasks
void display_update();

// Set backlight brightness (0-255)
void display_set_brightness(uint8_t brightness);

// Read encoder switch (on PCF8574 P5)
bool display_read_switch();

#endif // DISPLAY_H
