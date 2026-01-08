#include <Arduino.h>
#include <lvgl.h>
#include "ui.h"

// Global oil temperature variable (0-160°C)
int oil_temp = 0;

// Update UI based on oil temperature
void updateOilTempUI(int tempValue) {
    // Clamp oil temp value to valid range
    if (tempValue < 0) tempValue = 0;
    if (tempValue > 160) tempValue = 160;

    // Update label_oil_temp_value with integer value
    char valueStr[8];
    snprintf(valueStr, sizeof(valueStr), "%d", tempValue);
    lv_label_set_text(objects.label_oil_temp_value, valueStr);

    // Update arc_oil_temp (0-160 direct mapping)
    lv_arc_set_value(objects.arc_oil_temp, tempValue);

    // Update image_needle rotation
    // Value 0 = angle 450, each 10 value adds 168 degrees
    // Value 160 = angle 3138
    // Formula: angle = 450 + (tempValue * 16.8)
    int16_t needleAngle = 450 + (int16_t)(tempValue * 16.8f);
    // Clamp rotation to range 450-3138
    if (needleAngle < 450) needleAngle = 450;
    if (needleAngle > 3138) needleAngle = 3138;
    lv_img_set_angle(objects.image_needle, needleAngle);

    // Update image_oil_temp recolor based on temperature ranges
    uint32_t oilTempColor;
    if (tempValue < 60) {
        // 0-60: red
        oilTempColor = theme_colors[active_theme_index][COLOR_ID_COLOR_RED];
    } else if (tempValue < 80) {
        // 60-80: orange
        oilTempColor = theme_colors[active_theme_index][COLOR_ID_COLOR_ORANGE];
    } else if (tempValue < 130) {
        // 80-130: white
        oilTempColor = theme_colors[active_theme_index][COLOR_ID_COLOR_WHITE];
    } else if (tempValue < 140) {
        // 130-140: orange
        oilTempColor = theme_colors[active_theme_index][COLOR_ID_COLOR_ORANGE];
    } else {
        // 140-160: white
        oilTempColor = theme_colors[active_theme_index][COLOR_ID_COLOR_WHITE];
    }
    lv_obj_set_style_bg_img_recolor(objects.image_oil_temp, lv_color_hex(oilTempColor), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void setup() {
    // Initialize LVGL and display (user should add display initialization here)

    // Initialize UI
    ui_init();

    // Initial UI update with current oil temp value
    updateOilTempUI(oil_temp);
}

void loop() {
    // Handle LVGL tasks
    lv_timer_handler();

    // Update UI with current oil temp value
    updateOilTempUI(oil_temp);

    delay(5);
}
