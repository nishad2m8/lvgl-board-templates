#include <Arduino.h>
#include "display.h"
#include "encoder.h"
#include "led_ring.h"
#include "ui.h"
#include "screens.h"

// Power pins for 1.28 inch board
#define POWER_PIN_1 1
#define POWER_PIN_2 2

static char label_buf[16];

// Click detection
static unsigned long lastClickTime = 0;
static const unsigned long doubleClickTime = 300;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== CrowPanel 1.28inch Starting ===");

    // Initialize power pins
    pinMode(POWER_PIN_1, OUTPUT);
    digitalWrite(POWER_PIN_1, HIGH);
    pinMode(POWER_PIN_2, OUTPUT);
    digitalWrite(POWER_PIN_2, HIGH);

    // Initialize LED ring
    led_ring_init();

    // Initialize display and LVGL
    display_init();

    // Initialize encoder
    encoder_init();
    encoder_set_range(0, 100);

    // Initialize UI
    ui_init();

    // Start display (backlight on)
    display_start();

    // Start LED animation task
    led_ring_start_task();

    Serial.println("=== Ready ===");
}

void loop() {
    // Read encoder rotation
    encoder_event_t event = encoder_read();

    if (event == ENCODER_CW || event == ENCODER_CCW) {
        int value = encoder_get_value();
        lv_arc_set_value(objects.arc, value);
        snprintf(label_buf, sizeof(label_buf), "%d", value);
        lv_label_set_text(objects.label, label_buf);
    }

    // Handle switch clicks
    int clicks = encoder_get_click_count();
    if (clicks > 0 && millis() - lastClickTime > doubleClickTime) {
        if (clicks == 1) {
            Serial.println("[Switch] Single click");
            encoder_set_value(0);
            lv_arc_set_value(objects.arc, 0);
            lv_label_set_text(objects.label, "0");
        } else if (clicks >= 2) {
            Serial.println("[Switch] Double click");
            encoder_set_value(50);
            lv_arc_set_value(objects.arc, 50);
            lv_label_set_text(objects.label, "50");
        }
        encoder_clear_clicks();
        lastClickTime = millis();
    }

    // Update display
    display_update();
    ui_tick();
    delay(5);
}
