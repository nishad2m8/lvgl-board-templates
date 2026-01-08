#include "encoder.h"

// Encoder state
static int encoderValue = 0;
static int encoderMin = 0;
static int encoderMax = 100;
static int lastStateCLK = 0;

void encoder_init() {
    pinMode(ENCODER_A_PIN, INPUT);
    pinMode(ENCODER_B_PIN, INPUT);
    lastStateCLK = digitalRead(ENCODER_A_PIN);
    Serial.println("[Encoder] Initialized (A=42, B=4)");
}

encoder_event_t encoder_read() {
    encoder_event_t event = ENCODER_NONE;

    int currentStateCLK = digitalRead(ENCODER_A_PIN);

    if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
        if (digitalRead(ENCODER_B_PIN) != currentStateCLK) {
            // Counter-clockwise
            encoderValue--;
            if (encoderValue < encoderMin) encoderValue = encoderMin;
            event = ENCODER_CCW;
            Serial.printf("[Encoder] CCW -> %d\n", encoderValue);
        } else {
            // Clockwise
            encoderValue++;
            if (encoderValue > encoderMax) encoderValue = encoderMax;
            event = ENCODER_CW;
            Serial.printf("[Encoder] CW -> %d\n", encoderValue);
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
