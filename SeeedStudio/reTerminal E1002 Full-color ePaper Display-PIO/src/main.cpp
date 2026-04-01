#include <lvgl.h>
#include "e1002_display.h"
#include "ui.h"

e1002_driver_t e1002_driver;

void setup()
{
    e1002_display_init(&e1002_driver);
    ui_init();
}

void loop()
{
    lv_timer_handler();
    ui_tick();
    delay(5);

    if (e1002_display_should_refresh(&e1002_driver))
    {
        e1002_display_refresh(&e1002_driver);
    }
}
