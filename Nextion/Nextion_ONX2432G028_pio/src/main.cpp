#include <Arduino.h>

#include "display.h"
#include "ui.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nONX2432G028 PlatformIO / LVGL 9.5");

  display_init();
  ui_init();
}

void loop() {
  // lv_timer_handler() says when it wants to run next, but keep it sane.
  uint32_t idle = display_task();
  if (idle > 20) idle = 20;
  if (idle < 2) idle = 2;
  delay(idle);
}
