#include "LED_Fire.h"

static CRGB leds[LED_FIRE_NUM_LEDS];
static uint8_t heat[LED_FIRE_NUM_LEDS];

void LED_Fire_Init(uint8_t brightness)
{
  FastLED.addLeds<WS2812B, LED_FIRE_PIN, GRB>(leds, LED_FIRE_NUM_LEDS);
  FastLED.setBrightness(brightness);
  fill_solid(leds, LED_FIRE_NUM_LEDS, CRGB::Black);
  memset(heat, 0, sizeof(heat));
  FastLED.show();
}

// Fire2012 by Mark Kriegsman, adapted for this project
void LED_Fire_Update()
{
  // 1) Cool down every cell a little
  for (int i = 0; i < LED_FIRE_NUM_LEDS; i++) {
    uint8_t cooldown = random8(0, ((FIRE_COOLING * 10) / LED_FIRE_NUM_LEDS) + 2);
    heat[i] = qsub8(heat[i], cooldown);
  }

  // 2) Heat from each cell drifts up and diffuses
  for (int k = LED_FIRE_NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // 3) Randomly ignite new sparks near the bottom
  if (random8() < FIRE_SPARKING) {
    int y = random8(0, min<int>(LED_FIRE_NUM_LEDS, 7));
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // 4) Map from heat cells to LED colors
  for (int j = 0; j < LED_FIRE_NUM_LEDS; j++) {
    CRGB color = HeatColor(heat[j]);
    leds[j] = color;
  }

  FastLED.show();
}

