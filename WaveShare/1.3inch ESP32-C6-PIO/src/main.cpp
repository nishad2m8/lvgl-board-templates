#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "ui.h"
#include "LED_Fire.h"

// https://www.youtube.com/nishad2m8
// https://buymeacoffee.com/nishad2m8

void setup()
{       

  LCD_Init();

  // Set initial backlight brightness
  Set_Backlight(100);

  Lvgl_Init();

  // Adjust display rotation as needed 0, 90, 180, or 270 degrees
  Set_Rotation(90);

  // Start LED fire effect (brightness 64)
  LED_Fire_Init(64);

  // Initialize UI created with LVGL
  ui_init();

}

void loop()
{
  Timer_Loop();
  LED_Fire_Update();
  delay(5);
}
