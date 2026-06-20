#pragma once

#include <Arduino.h>
#include <lvgl.h>

void display_init();
void display_loop();
void display_set_brightness(uint8_t percent);
