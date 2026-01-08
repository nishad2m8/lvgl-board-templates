#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

// Encoder pins (from factory code)
#define ENCODER_A_PIN 42
#define ENCODER_B_PIN 4

// Encoder events
typedef enum {
    ENCODER_NONE,
    ENCODER_CW,
    ENCODER_CCW
} encoder_event_t;

// Initialize encoder
void encoder_init();

// Poll for encoder rotation (call in loop)
encoder_event_t encoder_read();

// Get current encoder value
int encoder_get_value();

// Set encoder value and limits
void encoder_set_value(int value);
void encoder_set_range(int min, int max);

#endif // ENCODER_H
