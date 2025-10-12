// Simple rotary encoder handler for ESP32
// Uses interrupt on pin A to detect direction and increments/decrements count.

#pragma once

#include <Arduino.h>

class Rotary {
public:
    Rotary(int pinA, int pinB, bool usePullups = true, bool reversed = false);

    void begin();
    void loop(); // Call regularly from main loop to print events

    long getCount() const { return count_; }
    void reset() { noInterrupts(); count_ = 0; delta_ = 0; interrupts(); }

private:
    static void IRAM_ATTR isrA_();
    void handleAChange_();

    int pinA_;
    int pinB_;
    bool usePullups_;
    bool reversed_;

    volatile int lastA_ = 0;
    volatile long delta_ = 0; // pending steps from ISR
    volatile long count_ = 0; // accumulated steps

    static Rotary* instance_;
};

