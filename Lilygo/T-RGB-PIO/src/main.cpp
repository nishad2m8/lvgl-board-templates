#include <LilyGo_RGBPanel.h>
#include <LV_Helper.h>
#include "ui.h"

LilyGo_RGBPanel panel;

void setup() {
    Serial.begin(115200);

     //** Four initialization methods */

    // Automatically determine the touch model to determine the initialization screen type. If touch is not available, it may fail.
    // bool rslt = panel.begin();

    // Specify 2.1-inch semicircular screen
    // https://www.lilygo.cc/products/t-rgb?variant=42407295877301
    // bool rslt = panel.begin(LILYGO_T_RGB_2_1_INCHES_HALF_CIRCLE);

    // Specified as a 2.1-inch full-circle screen
    // https://www.lilygo.cc/products/t-rgb
    // bool rslt = panel.begin(LILYGO_T_RGB_2_1_INCHES_FULL_CIRCLE);

    // Specified as a 2.8-inch full-circle screen
    // https://www.lilygo.cc/products/t-rgb?variant=42880799441077
    bool rslt = panel.begin(LILYGO_T_RGB_2_8_INCHES);

    if (!rslt) {
        while (1) {
            Serial.println("Error, failed to initialize T-RGB");
            delay(1000);
        }
    }

    // Call lvgl initialization
    beginLvglHelper(panel);

    Serial.println("BOOT: app started");

    // Initialize UI
    ui_init();

    // Turn on backlight (0-16)
    panel.setBrightness(16);
}

void loop() {
    // Handle LVGL tasks
    lv_timer_handler();

    delay(5);
}
