#include <Arduino.h>
#include "lvgl_setup.h"
#include <ui.h>

// https://www.youtube.com/nishad2m8
// https://buymeacoffee.com/nishad2m8

// Timer for LVGL tick
hw_timer_t *timer = NULL;

// Timer interrupt for LVGL tick
void IRAM_ATTR onTimer() {
    lv_tick_inc(5);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting setup...");
    
    // Initialize LVGL with landscape orientation
    lvgl_setup_init(ROTATION_LANDSCAPE_CW);
    
    // Initialize UI (from SquareLine Studio or custom UI)
    ui_init();
    
    // Setup hardware timer for LVGL tick (5ms)
    timer = timerBegin(0, 80, true);  // Timer 0, prescaler 80 (1MHz), count up
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 5000, true);  // 5ms interval
    timerAlarmEnable(timer);
    
    Serial.println("Setup complete!");
}

void loop() {
    // Handle LVGL tasks
    lv_timer_handler();
    delay(5);
}