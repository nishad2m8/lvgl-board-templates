#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "display_driver/display_driver.h"
#include "ui.h"

void setup() {
  Serial.begin(115200);

  pinMode(18, OUTPUT);

  // Initialize I2C
  Wire.begin(15, 16);
  delay(50);

  // GT911 touch controller power on timing --->0x5D
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(1, LOW);
  digitalWrite(2, LOW);
  delay(20);
  digitalWrite(2, HIGH);
  delay(100);
  pinMode(1, INPUT);

  // Initialize display hardware
  display_init();

  // Setup LVGL display and touch driver
  lvgl_display_setup();

  delay(100);

  // Turn on backlight
  pinMode(38, OUTPUT);
  digitalWrite(38, HIGH);

  // Clear screen
  gfx.fillScreen(TFT_BLACK);

  // Initialize UI
  ui_init();

  Serial.println("Setup done");
}

void loop() {
  lv_timer_handler(); // Let the GUI do its work
  delay(5);
}
