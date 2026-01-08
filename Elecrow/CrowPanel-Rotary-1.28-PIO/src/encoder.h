#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

// Encoder pins for 1.28 inch board
#define ENCODER_A_PIN 45    // CLK pin
#define ENCODER_B_PIN 42    // DT pin
#define SWITCH_PIN 41       // Button pin

// Encoder events
typedef enum {
    ENCODER_NONE,
    ENCODER_CW,
    ENCODER_CCW
} encoder_event_t;

// Initialize encoder and switch
void encoder_init();

// Poll for encoder rotation (call in loop)
encoder_event_t encoder_read();

// Get current encoder value
int encoder_get_value();

// Set encoder value and limits
void encoder_set_value(int value);
void encoder_set_range(int min, int max);

// Switch handling
bool encoder_switch_pressed();
int encoder_get_click_count();
void encoder_clear_clicks();

#endif // ENCODER_H
