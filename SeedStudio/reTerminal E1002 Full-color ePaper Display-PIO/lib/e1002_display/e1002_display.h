#ifndef LVGL_DISPLAY_H
#define LVGL_DISPLAY_H

#include <TFT_eSPI.h>

typedef struct {
    EPaper *epd;
    uint32_t flush_scheduled_time;
    bool flush_scheduled;
} e1002_driver_t;

void e1002_display_init(e1002_driver_t *drv);

void e1002_display_refresh(e1002_driver_t *drv);

void e1002_display_schedule_refresh(e1002_driver_t *drv);

bool e1002_display_should_refresh(e1002_driver_t *drv);

#endif