#include <Arduino.h>
#include "display.h"
#include "encoder.h"
#include "ui.h"
#include "screens.h"

static lv_obj_t *arc = NULL;

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("\n=== CrowPanel 2.1inch Starting ===");

    display_init();
    encoder_init();
    encoder_set_range(0, 100);

    ui_init();
    display_start();

    // Find the arc widget (child index 1 of main screen)
    // Use objects.main directly since lv_scr_act() may not be ready during screen transition
    arc = lv_obj_get_child(objects.main, 1);

    Serial.println("=== Ready ===");
}

void loop() {
    encoder_event_t event = encoder_read();

    if (event != ENCODER_NONE && arc) {
        int value = encoder_get_value();
        lv_arc_set_value(arc, value);
        Serial.printf("[Arc] value=%d\n", value);
    }

    display_update();
    ui_tick();
    delay(5);
}
