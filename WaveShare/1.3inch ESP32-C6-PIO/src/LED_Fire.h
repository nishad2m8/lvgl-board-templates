#pragma once

#include <Arduino.h>
#include <FastLED.h>

// Pin and LED count defaults.
// If the project already defines WS2812B_PIN / WS2812B_NUM_LEDS, reuse them.
#ifndef LED_FIRE_PIN
#  ifdef WS2812B_PIN
#    define LED_FIRE_PIN WS2812B_PIN
#  else
#    define LED_FIRE_PIN 8
#  endif
#endif

#ifndef LED_FIRE_NUM_LEDS
#  ifdef WS2812B_NUM_LEDS
#    define LED_FIRE_NUM_LEDS WS2812B_NUM_LEDS
#  else
#    define LED_FIRE_NUM_LEDS 16
#  endif
#endif

// Fire parameters (can be overridden before including this header)
#ifndef FIRE_COOLING
#define FIRE_COOLING  55
#endif

#ifndef FIRE_SPARKING
#define FIRE_SPARKING 120
#endif

void LED_Fire_Init(uint8_t brightness = 64);
void LED_Fire_Update();

