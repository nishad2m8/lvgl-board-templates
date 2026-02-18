/**
 * Waveshare ESP32-S3 e-Paper 3.97" - LVGL 9.3.x Hello World
 *
 * Board: ESP32-S3-WROOM-1-N16R8 (16MB Flash, 8MB PSRAM)
 * Display: 800x480 monochrome e-Paper with fast & partial refresh
 */

#include <Arduino.h>
#include <math.h>
#include <lvgl.h>
#include "ui.h"
#include "images.h"
#include "epd_display.h"
// #include "sensors.h"


// youtube: https://www.youtube.com/nishad2m8
// buymeacoffee: https://buymeacoffee.com/nishad2m8

// Arduino setup
void setup()
{
    Serial.begin(115200);
    while (!Serial && millis() < 3000) { delay(10); }
    delay(500);

    epd_hardware_init();
    epd_lvgl_setup();

    Serial.println("[HW] Hardware initialization complete.");

    // Monochrome theme
    lv_display_t *disp = lv_display_get_default();
    lv_theme_t *th = lv_theme_mono_init(disp, false, &lv_font_montserrat_14);
    lv_display_set_theme(disp, th);

    // UI initialization Created using EEZ Studio
    ui_init();

    Serial.println("[APP] UI loaded. First render will appear on e-paper shortly...");
}

// Arduino loop
#define LVGL_TASK_PERIOD  5  // ms between lv_timer_handler calls

static unsigned long last_sensor_poll = 0;
static const unsigned long SENSOR_POLL_INTERVAL = 5000;

void loop()
{
    lv_timer_handler();
    ui_tick();

    delay(LVGL_TASK_PERIOD);
}
