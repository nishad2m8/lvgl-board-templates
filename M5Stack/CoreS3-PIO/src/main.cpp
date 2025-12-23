#include <Arduino.h>
#include "M5GFX.h"
#include "M5Unified.h"
#include "lvgl.h"
#include "m5gfx_lvgl.h"
#include "ui.h"

void setup() {
    M5.begin();
    M5.Display.setBrightness(100);

    lv_init();
    m5gfx_lvgl_init();

    ui_init();
}

void loop() {
    lv_timer_handler();
    delay(10);
}
