#pragma once

#include <lvgl.h>

// Panel + touch bring-up, and the LVGL glue. Call display_init() before you
// create any object, then pump display_task() from loop().

void display_init();
uint32_t display_task();
void display_set_backlight(bool on);
lv_display_t *display_get();
