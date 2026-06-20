#include <Arduino.h>
#include "display.h"
#include "ui.h"

void setup()
{
    Serial.begin(115200);

    display_init();
    ui_init();

    Serial.println("Setup done");
}

void loop()
{
    display_loop();
    ui_tick();
    delay(5);
}
