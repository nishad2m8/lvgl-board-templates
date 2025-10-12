// Rotary.cpp - simple rotary encoder with ISR on pin A

#include "Rotary.h"

Rotary* Rotary::instance_ = nullptr;

Rotary::Rotary(int pinA, int pinB, bool usePullups, bool reversed)
    : pinA_(pinA), pinB_(pinB), usePullups_(usePullups), reversed_(reversed) {}

void Rotary::begin() {
    instance_ = this;

    pinMode(pinA_, usePullups_ ? INPUT_PULLUP : INPUT);
    pinMode(pinB_, usePullups_ ? INPUT_PULLUP : INPUT);

    lastA_ = digitalRead(pinA_);
    attachInterrupt(digitalPinToInterrupt(pinA_), isrA_, CHANGE);
}

void Rotary::loop() {
    long d = 0;
    noInterrupts();
    d = delta_;
    delta_ = 0;
    interrupts();

    while (d > 0) {
        count_++;
        Serial.printf("Knob CW, count: %ld\n", count_);
        d--;
    }
    while (d < 0) {
        count_--;
        Serial.printf("Knob CCW, count: %ld\n", count_);
        d++;
    }
}

void IRAM_ATTR Rotary::isrA_() {
    if (instance_) instance_->handleAChange_();
}

void IRAM_ATTR Rotary::handleAChange_() {
    int a = digitalRead(pinA_);
    if (a == lastA_) return; // no actual change
    lastA_ = a;
    int b = digitalRead(pinB_);

    // Direction: on A change, if A == B then CW else CCW (may vary per wiring)
    int step = (a == b) ? 1 : -1;
    if (reversed_) step = -step;
    delta_ += step;
}
