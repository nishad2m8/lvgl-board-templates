#include <Arduino.h>
#include <lvgl.h>
#include "ui.h"

// Global oil pressure variable (0-6 bar)
float oil_pressure = 0;

// Update UI based on oil pressure
void updateOilPressureUI(float pressureValue) {
    // Clamp oil pressure value to valid range
    if (pressureValue < 0) pressureValue = 0;
    if (pressureValue > 6) pressureValue = 6;

    // Update label_oil_pressure_value with pressure value
    char valueStr[8];
    snprintf(valueStr, sizeof(valueStr), "%.1f", pressureValue);
    lv_label_set_text(objects.label_oil_pressure_value, valueStr);

    // Update arc_oil_temp (remap 0-6 to 0-60)
    int arcValue = (int)(pressureValue * 10);
    lv_arc_set_value(objects.arc_oil_temp, arcValue);

    // Update image_needle rotation
    // Pressure 0 = angle 450, Pressure 1 = angle 900, Pressure 6 = angle 3150
    // Formula: angle = 450 + (pressureValue * 450)
    int16_t needleAngle = 450 + (int16_t)(pressureValue * 450.0f);
    // Clamp rotation to range 450-3150 (CW)
    if (needleAngle < 450) needleAngle = 450;
    if (needleAngle > 3150) needleAngle = 3150;
    lv_img_set_angle(objects.image_needle, needleAngle);

    // Update image_oil_temp recolor based on pressure ranges
    uint32_t oilPressureColor;
    if (pressureValue < 1.5f) {
        // 0-1.5: red
        oilPressureColor = theme_colors[active_theme_index][COLOR_ID_COLOR_RED];
    } else if (pressureValue < 2.0f) {
        // 1.5-2: orange
        oilPressureColor = theme_colors[active_theme_index][COLOR_ID_COLOR_ORANGE];
    } else {
        // 2-6: white
        oilPressureColor = theme_colors[active_theme_index][COLOR_ID_COLOR_WHITE];
    }
    lv_obj_set_style_bg_img_recolor(objects.image_oil_temp, lv_color_hex(oilPressureColor), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void setup() {
    // Initialize LVGL and display (user should add display initialization here)

    // Initialize UI
    ui_init();

    // Initial UI update with current oil pressure value
    updateOilPressureUI(oil_pressure);
}

void loop() {
    // Handle LVGL tasks
    lv_timer_handler();

    // Update UI with current oil pressure value
    updateOilPressureUI(oil_pressure);

    delay(5);
}
