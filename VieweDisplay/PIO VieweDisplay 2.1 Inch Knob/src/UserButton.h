// Simple debounced button handler

#pragma once

#include <Arduino.h>

class UserButton {
public:
    // activeLow = true for buttons wired to GND with pull-up
    explicit UserButton(int pin, bool activeLow = true, uint16_t debounceMs = 30, uint16_t longPressMs = 700);

    void begin();
    void loop(); // Call regularly from main loop to print events

private:
    int pin_;
    bool activeLow_;
    uint16_t debounceMs_;
    uint16_t longPressMs_;

    bool stableState_ = false; // debounced level (pressed = true)
    bool lastRead_ = false;
    unsigned long lastChangeMs_ = 0;
    unsigned long pressStartMs_ = 0;
    bool longPressReported_ = false;
};

