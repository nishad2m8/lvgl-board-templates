#include "display_driver.h"
#include "pins_config.h"
#include <Arduino.h>

// LGFX instance
LGFX gfx;

// LVGL display buffers
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf;
static lv_color_t *buf1;

// Touch coordinates
static uint16_t touchX, touchY;

// Display flush callback for LVGL
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  if (gfx.getStartCount() > 0) {
    gfx.endWrite();
  }
  gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);

  lv_disp_flush_ready(disp);  // Tell lvgl that the refresh is complete
}

// Touchpad read callback for LVGL
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  data->state = LV_INDEV_STATE_REL;
  if (gfx.getTouch(&touchX, &touchY)) {
    data->state = LV_INDEV_STATE_PR;
    // Set coordinates (the Y-axis of the screen on Longxian is opposite, and the X-axis of the screen on Puyang is opposite)
    data->point.x = LCD_H_RES - touchX;
    data->point.y = touchY; // After rotation, the Y-axis is reversed

    Serial.print("Data x ");
    Serial.println(data->point.x);
    Serial.print("Data y ");
    Serial.println(data->point.y);
  }
}

// Initialize display hardware
void display_init() {
  gfx.init();
  gfx.initDMA();
  gfx.startWrite();
  gfx.fillScreen(TFT_BLACK);
}

// Initialize LVGL display driver
void lvgl_display_setup() {
  // Initialize LVGL
  lv_init();

  // Allocate buffers in PSRAM
  size_t buffer_size = sizeof(lv_color_t) * LCD_H_RES * LCD_V_RES;
  buf = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  buf1 = (lv_color_t *)heap_caps_malloc(buffer_size, MALLOC_CAP_SPIRAM);
  assert(buf);
  assert(buf1);

  lv_disp_draw_buf_init(&draw_buf, buf, buf1, LCD_H_RES * LCD_V_RES);

  // Initialize display driver
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LCD_H_RES;
  disp_drv.ver_res = LCD_V_RES;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // Initialize input device driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);
}
