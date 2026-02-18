#include "display.h"
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "lv_conf.h"
#include "TouchDrvCSTXXX.hpp"
#include <Arduino.h>
#include <Wire.h>

#define EXAMPLE_LVGL_TICK_PERIOD_MS 2

static lv_disp_draw_buf_t draw_buf;

static Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_CS /* CS */, LCD_SCLK /* SCK */, LCD_SDIO0 /* SDIO0 */, LCD_SDIO1 /* SDIO1 */,
  LCD_SDIO2 /* SDIO2 */, LCD_SDIO3 /* SDIO3 */);

static Arduino_CO5300 *gfx = new Arduino_CO5300(
  bus, LCD_RESET /* RST */, 0 /* rotation */, LCD_WIDTH /* width */, LCD_HEIGHT /* height */, 6, 0, 0, 0);

static TouchDrvCST92xx touch;
static int16_t x[5], y[5];
static volatile bool isPressed = false;

#if LV_USE_LOG != 0
static void my_print(const char *msg) {
  Serial.printf(msg);
  Serial.flush();
}
#endif

static void example_lvgl_rounder_cb(struct _lv_disp_drv_t *disp_drv, lv_area_t *area) {
  if (area->x1 % 2 != 0) area->x1--;
  if (area->y1 % 2 != 0) area->y1--;
  if (area->x2 % 2 == 0) area->x2++;
  if (area->y2 % 2 == 0) area->y2++;
}

static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}

static void lvgl_tick_cb(void *arg) {
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  if (isPressed) {
    uint8_t touched = touch.getPoint(x, y, touch.getSupportTouchPoint());
    if (touched) {
      isPressed = false;
      data->state = LV_INDEV_STATE_PR;
      data->point.x = x[0];
      data->point.y = y[0];
      Serial.print("Touch x:");
      Serial.print(x[0]);
      Serial.print(" y:");
      Serial.println(y[0]);
    } else {
      data->state = LV_INDEV_STATE_REL;
    }
  }
}

void display_init() {
  Wire.begin(IIC_SDA, IIC_SCL);

  /* Touch reset sequence */
  digitalWrite(TP_RST, LOW);
  delay(30);
  digitalWrite(TP_RST, HIGH);
  delay(50);
  delay(1000);

  Wire.begin(IIC_SDA, IIC_SCL);

  /* Initialize touch */
  touch.setPins(TP_RST, TP_INT);
  bool result = touch.begin(Wire, 0x5A, IIC_SDA, IIC_SCL);
  if (result == false) {
    Serial.println("Touch is not online...");
    while (1) delay(1000);
  }
  Serial.print("Touch Model: ");
  Serial.println(touch.getModelName());
  touch.sleep();
  touch.reset();
  touch.setMaxCoordinates(LCD_WIDTH, LCD_HEIGHT);
  touch.setMirrorXY(true, true);
  attachInterrupt(TP_INT, []() { isPressed = true; }, FALLING);

  /* Initialize display */
  gfx->begin();
  gfx->setBrightness(200);

  uint32_t screenWidth = gfx->width();
  uint32_t screenHeight = gfx->height();

  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print);
#endif

  /* Double-buffered DMA allocation */
  lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 4 * sizeof(lv_color_t), MALLOC_CAP_DMA);
  lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 4 * sizeof(lv_color_t), MALLOC_CAP_DMA);
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 4);

  /* Initialize the display driver */
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.rounder_cb = example_lvgl_rounder_cb;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.sw_rotate = 1;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the touch input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  /* Setup LVGL tick timer */
  const esp_timer_create_args_t lvgl_tick_timer_args = {
    .callback = &lvgl_tick_cb,
    .name = "lvgl_tick"
  };

  esp_timer_handle_t lvgl_tick_timer = NULL;
  esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, EXAMPLE_LVGL_TICK_PERIOD_MS * 1000);
}
