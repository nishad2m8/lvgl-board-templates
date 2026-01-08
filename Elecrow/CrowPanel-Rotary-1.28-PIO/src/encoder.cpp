#include "encoder.h"

// Encoder state
static int encoderValue = 0;
static int encoderMin = 0;
static int encoderMax = 100;
static int lastStateCLK = 0;

// Switch state (using ISR)
static volatile unsigned long lastPressTime = 0;
static volatile bool pressFlag = false;
static volatile int clickCount = 0;
static const unsigned long debounceTime = 20;

// ISR for button press
void IRAM_ATTR buttonISR() {
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > debounceTime) {
        if (digitalRead(SWITCH_PIN)) {
            pressFlag = false;
        } else {
            pressFlag = true;
            lastPressTime = interruptTime;
            clickCount++;
        }
    }
    lastInterruptTime = interruptTime;
}

void encoder_init() {
    pinMode(ENCODER_A_PIN, INPUT);
    pinMode(ENCODER_B_PIN, INPUT);
    pinMode(SWITCH_PIN, INPUT_PULLUP);

    // Attach interrupt for switch
    attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), buttonISR, CHANGE);

    lastStateCLK = digitalRead(ENCODER_A_PIN);
    Serial.println("[Encoder] Initialized (A=45, B=42, SW=41)");
}

encoder_event_t encoder_read() {
    encoder_event_t event = ENCODER_NONE;

    int currentStateCLK = digitalRead(ENCODER_A_PIN);

    if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
        if (digitalRead(ENCODER_B_PIN) != currentStateCLK) {
            // Clockwise rotation
            encoderValue++;
            if (encoderValue > encoderMax) encoderValue = encoderMax;
            event = ENCODER_CW;
            Serial.printf("[Encoder] CW -> %d\n", encoderValue);
        } else {
            // Counter-clockwise rotation
            encoderValue--;
            if (encoderValue < encoderMin) encoderValue = encoderMin;
            event = ENCODER_CCW;
            Serial.printf("[Encoder] CCW -> %d\n", encoderValue);
        }
    }
    lastStateCLK = currentStateCLK;

    return event;
}

int encoder_get_value() {
    return encoderValue;
}

void encoder_set_value(int value) {
    encoderValue = value;
    if (encoderValue < encoderMin) encoderValue = encoderMin;
    if (encoderValue > encoderMax) encoderValue = encoderMax;
}

void encoder_set_range(int min, int max) {
    encoderMin = min;
    encoderMax = max;
}

bool encoder_switch_pressed() {
    return pressFlag;
}

int encoder_get_click_count() {
    return clickCount;
}

void encoder_clear_clicks() {
    clickCount = 0;
}
