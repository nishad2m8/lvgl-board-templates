// UserButton.cpp - simple debounced button with press/release and long-press print

#include "UserButton.h"

UserButton::UserButton(int pin, bool activeLow, uint16_t debounceMs, uint16_t longPressMs)
    : pin_(pin), activeLow_(activeLow), debounceMs_(debounceMs), longPressMs_(longPressMs) {}

void UserButton::begin() {
    if (activeLow_) {
        pinMode(pin_, INPUT_PULLUP);
    } else {
        pinMode(pin_, INPUT);
    }
    bool raw = digitalRead(pin_);
    bool pressed = activeLow_ ? !raw : raw;
    stableState_ = pressed;
    lastRead_ = pressed;
    lastChangeMs_ = millis();
}

void UserButton::loop() {
    bool raw = digitalRead(pin_);
    bool pressed = activeLow_ ? !raw : raw;
    unsigned long now = millis();

    if (pressed != lastRead_) {
        lastRead_ = pressed;
        lastChangeMs_ = now;
    }

    // Debounce
    if ((now - lastChangeMs_) >= debounceMs_ && pressed != stableState_) {
        stableState_ = pressed;
        if (stableState_) {
            pressStartMs_ = now;
            longPressReported_ = false;
            Serial.println("Button pressed");
        } else {
            Serial.println("Button released");
        }
    }

    // Long press report (once)
    if (stableState_ && !longPressReported_ && (now - pressStartMs_) >= longPressMs_) {
        longPressReported_ = true;
        Serial.println("Button long press");
    }
}

