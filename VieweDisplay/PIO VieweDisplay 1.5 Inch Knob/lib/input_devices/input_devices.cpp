/*
 * Initialize knob and button, printing events to Serial only.
 */

#include <Arduino.h>
#include <ESP_Knob.h>
#include <Button.h>
#include "input_devices.h"
#include "ESP_Panel_Board_Supported.h"

static ESP_Knob *s_knob = nullptr;
static Button  *s_button = nullptr;

// Map library's "left" event to Clockwise (CW) for this hardware
static void on_knob_cw(int count, void *usr_data) {
    (void)usr_data;
    Serial.printf("Knob CW, count=%d\n", count);
}

// Map library's "right" event to Counter-Clockwise (CCW) for this hardware
static void on_knob_ccw(int count, void *usr_data) {
    (void)usr_data;
    Serial.printf("Knob CCW, count=%d\n", count);
}

static void on_button_single(void *button_handle, void *usr_data) {
    (void)button_handle;
    (void)usr_data;
    Serial.println("Button Single Click");
}

void input_init() {
    // Knob
    if (!s_knob) {
        s_knob = new ESP_Knob(GPIO_NUM_KNOB_PIN_A, GPIO_NUM_KNOB_PIN_B);
        s_knob->begin();
        s_knob->attachLeftEventCallback(on_knob_cw);
        s_knob->attachRightEventCallback(on_knob_ccw);
    }

    // Button
    if (!s_button) {
        s_button = new Button((gpio_num_t)GPIO_BUTTON_PIN, false);
        s_button->attachSingleClickEventCb(&on_button_single, nullptr);
    }
}

void input_deinit() {
    if (s_button) {
        s_button->del();
        s_button = nullptr;
    }
    if (s_knob) {
        s_knob->del();
        delete s_knob;
        s_knob = nullptr;
    }
}
