#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <lvgl.h>
#include "LovyanGFX_Driver.h"

// External access to gfx object
extern LGFX gfx;

// Display and touch initialization
void display_init();
void touch_init();
void lvgl_display_setup();

// LVGL callbacks
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

#endif // DISPLAY_DRIVER_H
