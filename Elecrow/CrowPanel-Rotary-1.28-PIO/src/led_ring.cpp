#include "led_ring.h"
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel led(LED_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);
static TaskHandle_t ledTaskHandle = NULL;

// LED test task - runs the factory demo animation
static void ledTestTask(void *pvParameters) {
    uint8_t ledCount = 0;
    int8_t ledBrightness = 0;

    while (1) {
        led.clear();
        led.show();

        // Five circles of white flowing water lights
        while (ledCount++ < 5) {
            for (int i = 0; i < 5; i++) {
                led.setPixelColor(i, led.Color(255, 255, 255));
                led.show();
                vTaskDelay(pdMS_TO_TICKS(250));
                led.clear();
                led.show();
            }
        }
        ledCount = 0;

        // All the lights flash in rapid succession in various colors simultaneously
        for (int i = 0; i < 5; i++) {
            led.setPixelColor(0, led.Color(255, 0, 0));
            led.setPixelColor(1, led.Color(0, 255, 0));
            led.setPixelColor(2, led.Color(0, 0, 255));
            led.setPixelColor(3, led.Color(255, 255, 0));
            led.setPixelColor(4, led.Color(130, 0, 255));
            led.show();
            vTaskDelay(pdMS_TO_TICKS(100));
            led.clear();
            led.show();
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        // Colorful flowing lights in 5 circles
        while (ledCount < 5) {
            for (int i = 0; i < 5; i++) {
                led.clear();
                switch (i) {
                    case 0: led.setPixelColor(i, led.Color(255, 0, 0)); break;
                    case 1: led.setPixelColor(i, led.Color(0, 255, 0)); break;
                    case 2: led.setPixelColor(i, led.Color(0, 0, 255)); break;
                    case 3: led.setPixelColor(i, led.Color(255, 255, 0)); break;
                    case 4: led.setPixelColor(i, led.Color(130, 0, 255)); break;
                }
                led.show();
                vTaskDelay(pdMS_TO_TICKS(250));
            }
            ledCount++;
        }
        ledCount = 0;

        // All the lights flash in a slow, colored pattern simultaneously
        for (int i = 0; i < 5; i++) {
            led.setPixelColor(0, led.Color(255, 0, 0));
            led.setPixelColor(1, led.Color(0, 255, 0));
            led.setPixelColor(2, led.Color(0, 0, 255));
            led.setPixelColor(3, led.Color(255, 255, 0));
            led.setPixelColor(4, led.Color(130, 0, 255));
            led.show();
            vTaskDelay(pdMS_TO_TICKS(250));
            led.clear();
            led.show();
            vTaskDelay(pdMS_TO_TICKS(250));
        }

        // Colorful breathing light, breathing 5 times
        led.setPixelColor(0, led.Color(255, 0, 0));
        led.setPixelColor(1, led.Color(0, 255, 0));
        led.setPixelColor(2, led.Color(0, 0, 255));
        led.setPixelColor(3, led.Color(255, 255, 0));
        led.setPixelColor(4, led.Color(130, 0, 255));
        while (ledCount++ < 10) {
            for (ledBrightness = 0; ledBrightness <= 25; ledBrightness++) {
                led.setBrightness(ledBrightness);
                led.setPixelColor(0, led.Color(255, 0, 0));
                led.setPixelColor(1, led.Color(0, 255, 0));
                led.setPixelColor(2, led.Color(0, 0, 255));
                led.setPixelColor(3, led.Color(255, 255, 0));
                led.setPixelColor(4, led.Color(130, 0, 255));
                led.show();
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            for (; ledBrightness >= 0; ledBrightness--) {
                led.setBrightness(ledBrightness);
                led.show();
                vTaskDelay(pdMS_TO_TICKS(50));
            }
            ledCount++;
        }

        ledCount = 0;
        ledBrightness = 0;
        led.setBrightness(25);
        led.clear();
        led.show();

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void led_ring_init() {
    // Initialize power light
    pinMode(POWER_LIGHT_PIN, OUTPUT);
    digitalWrite(POWER_LIGHT_PIN, LOW);

    // Initialize NeoPixel strip
    led.begin();
    led.setBrightness(25);
    led.clear();
    led.show();

    Serial.println("[LED Ring] Initialized (5 LEDs on pin 48)");
}

void led_ring_start_task() {
    if (ledTaskHandle == NULL) {
        xTaskCreatePinnedToCore(
            ledTestTask,
            "LED Test",
            2048,
            NULL,
            1,
            &ledTaskHandle,
            0  // Run on core 0
        );
        Serial.println("[LED Ring] Task started");
    }
}

void led_ring_stop_task() {
    if (ledTaskHandle != NULL) {
        vTaskDelete(ledTaskHandle);
        ledTaskHandle = NULL;
        led.clear();
        led.show();
        Serial.println("[LED Ring] Task stopped");
    }
}

void led_ring_set_color(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < LED_NUM; i++) {
        led.setPixelColor(i, led.Color(r, g, b));
    }
}

void led_ring_set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < LED_NUM) {
        led.setPixelColor(index, led.Color(r, g, b));
    }
}

void led_ring_set_brightness(uint8_t brightness) {
    led.setBrightness(brightness);
}

void led_ring_clear() {
    led.clear();
}

void led_ring_show() {
    led.show();
}

void power_light_on() {
    digitalWrite(POWER_LIGHT_PIN, HIGH);
}

void power_light_off() {
    digitalWrite(POWER_LIGHT_PIN, LOW);
}
