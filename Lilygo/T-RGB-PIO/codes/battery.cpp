#include <Arduino.h>
#include <lvgl.h>
#include "ui.h"

// Global battery voltage variable (0-16V)
float battery = 0.0f;

// Update UI based on battery voltage
void updateBatteryUI(float batteryValue) {
    // Clamp battery value to valid range
    if (batteryValue < 0.0f) batteryValue = 0.0f;
    if (batteryValue > 16.0f) batteryValue = 16.0f;

    // Update label_battery_value with decimal value
    char valueStr[8];
    snprintf(valueStr, sizeof(valueStr), "%.1f", batteryValue);
    lv_label_set_text(objects.label_battery_value, valueStr);

    // Update arc_battery (remap 0-16 to 0-160)
    int arcValue = (int)(batteryValue * 10.0f);
    lv_arc_set_value(objects.arc_battery, arcValue);

    // Update image_needle rotation
    // Value 0 = angle 0, Value 1 = angle 187, Value 16 = angle 2992
    // Linear mapping: angle = batteryValue * 187
    int16_t needleAngle = (int16_t)(batteryValue * 187.0f);
    // Clamp rotation to range 0-2992
    if (needleAngle < 0) needleAngle = 0;
    if (needleAngle > 2992) needleAngle = 2992;
    lv_img_set_angle(objects.image_needle, needleAngle);

    // Update image_battery recolor based on voltage ranges
    // Using theme_colors: [0]=RED, [1]=ORANGE, [2]=GREEN
    uint32_t batteryColor;
    if (batteryValue < 9.0f) {
        // 0-9: red
        batteryColor = theme_colors[active_theme_index][COLOR_ID_COLOR_RED];
    } else if (batteryValue < 11.5f) {
        // 9-11.5: orange
        batteryColor = theme_colors[active_theme_index][COLOR_ID_COLOR_ORANGE];
    } else if (batteryValue < 14.5f) {
        // 11.5-14.5: green
        batteryColor = theme_colors[active_theme_index][COLOR_ID_COLOR_GREEN];
    } else {
        // 14.5-16: orange
        batteryColor = theme_colors[active_theme_index][COLOR_ID_COLOR_ORANGE];
    }
    lv_obj_set_style_bg_img_recolor(objects.image_battery, lv_color_hex(batteryColor), LV_PART_MAIN | LV_STATE_DEFAULT);
}

void setup() {
    // Initialize LVGL and display (user should add display initialization here)

    // Initialize UI
    ui_init();

    // Initial UI update with current battery value
    updateBatteryUI(battery);
}

void loop() {
    // Handle LVGL tasks
    lv_timer_handler();

    // Update UI with current battery value
    updateBatteryUI(battery);

    delay(5);
}
