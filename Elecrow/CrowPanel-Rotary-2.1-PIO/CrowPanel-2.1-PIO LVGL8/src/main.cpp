#include <Arduino.h>
#include "display.h"
#include "encoder.h"
#include "ui.h"
#include "screens.h"

static char label_buf[16];

// Switch debounce
static unsigned long lastPressTime = 0;
static int clickCount = 0;
static bool lastSwitchState = false;
static const unsigned long debounceTime = 50;
static const unsigned long doubleClickTime = 300;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== CrowPanel 2.1inch Starting ===");

    display_init();
    encoder_init();
    encoder_set_range(0, 100);

    ui_init();
    display_start();  // Backlight ON after ui_init (factory sequence)

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

    // Read switch from PCF8574
    bool switchPressed = display_read_switch();

    // Detect press edge
    if (switchPressed && !lastSwitchState) {
        unsigned long now = millis();
        if (now - lastPressTime > debounceTime) {
            clickCount++;
            lastPressTime = now;
            Serial.printf("[Switch] Press detected, count=%d\n", clickCount);
        }
    }
    lastSwitchState = switchPressed;

    // Handle click timeout
    if (clickCount > 0 && millis() - lastPressTime > doubleClickTime) {
        if (clickCount == 1) {
            Serial.println("[Switch] Single click");
            encoder_set_value(0);
            lv_arc_set_value(objects.arc, 0);
            lv_label_set_text(objects.label, "0");
        } else if (clickCount >= 2) {
            Serial.println("[Switch] Double click");
            encoder_set_value(50);
            lv_arc_set_value(objects.arc, 50);
            lv_label_set_text(objects.label, "50");
        }
        clickCount = 0;
    }

    display_update();
    ui_tick();
    delay(5);
}
