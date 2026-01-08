#ifndef LED_RING_H
#define LED_RING_H

#include <Arduino.h>

// LED Ring configuration for 1.28 inch board
#define LED_PIN 48
#define LED_NUM 5
#define POWER_LIGHT_PIN 40

// Initialize LED ring and power light
void led_ring_init();

// Start the LED animation task (runs in background)
void led_ring_start_task();

// Stop the LED animation task
void led_ring_stop_task();

// Set all LEDs to a single color
void led_ring_set_color(uint8_t r, uint8_t g, uint8_t b);

// Set a specific LED color
void led_ring_set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

// Set LED brightness (0-255)
void led_ring_set_brightness(uint8_t brightness);

// Clear all LEDs
void led_ring_clear();

// Show the LED changes
void led_ring_show();

// Power light control
void power_light_on();
void power_light_off();

#endif // LED_RING_H
