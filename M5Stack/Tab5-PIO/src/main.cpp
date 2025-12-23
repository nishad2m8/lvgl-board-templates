// This project is based on https://github.com/nikthefix/M5Stack_Tab5_Arduino_Basic_LVGL_Demo by nikthefix

#include "display_driver.h"
#include "ui.h"

void setup()
{
    Serial.begin(115200);
    display_init();
    ui_init();
}

void loop()
{
    lv_timer_handler();
    delay(1);
}
