#include <M5Cardputer.h>  // Include M5Cardputer library for M5Stack-specific functionality
#include <lvgl.h>
#include "lvgl_setup.h"
#include <ui.h>

// https://www.youtube.com/nishad2m8
// https://buymeacoffee.com/nishad2m8

void setup() {
    Serial.begin(115200);

    // M5Cardputer setup
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Keyboard.begin();
    cfg.serial_baudrate = 115200;
    M5.begin(cfg);

    M5.Display.setRotation(1);  // Rotate screen for landscape orientation
    M5.Display.setBrightness(128);  // Set brightness to half

    // LVGL setup
    lvgl_setup();
    
    ui_init();      // Initialize UI from LVGL helper

    // Create a FreeRTOS task for handling LVGL ticks
    xTaskCreatePinnedToCore(lv_tick_task, "lv_tick_task", 4096, NULL, 5, NULL, 1);
}

void loop()
{
    M5Cardputer.update();   // Update M5Cardputer state

    lv_timer_handler();     // Handle GUI updates
    delay(5);               // Small delay to control loop speed
}
