#include <Arduino.h>
#include "display.h"
#include "ui.h"

// youtube: https://www.youtube.com/nishad2m8
// buymeacoffee: https://buymeacoffee.com/nishad2m8

void setup() {
  Serial.begin(115200);

  display_init();

  // UI initialization Created using EEZ Studio
  ui_init();

  Serial.println("Setup done");
}

void loop() {
  lv_timer_handler();
  ui_tick();
  delay(5);
}
