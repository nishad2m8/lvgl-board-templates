#include <Arduino.h>
#include <M5Dial.h>
#include <lvgl.h>
#include "M5Dial-LVGL.h"
#include "ui.h"

void setup() {
    Serial.begin(115200);

    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    m5dial_lvgl_init();

    ui_init();
}

void loop() {
    m5dial_lvgl_next();
    ui_tick();
}
