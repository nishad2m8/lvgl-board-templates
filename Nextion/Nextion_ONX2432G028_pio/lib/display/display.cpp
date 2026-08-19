#include "display.h"

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Arduino_GFX_Library.h>
#include <Adafruit_CST8XX.h>
#include <Adafruit_PCF8574.h>

#include "BoardConfig.h"

static Arduino_DataBus *bus = new Arduino_HWSPI(
    LCD_DC_PIN, LCD_CS_PIN, LCD_SCLK_PIN, LCD_MOSI_PIN, LCD_MISO_PIN, &SPI, true);
static Arduino_GFX *gfx = new Arduino_ST7789(
    bus, GFX_NOT_DEFINED, 0, false, LCD_H_RES, LCD_V_RES);

static Adafruit_CST8XX touch;
static Adafruit_PCF8574 ioExpander;

static lv_display_t *disp = nullptr;
static lv_indev_t *indev = nullptr;

// 40-line partial buffer, 19k. Bytes, not lv_color_t: that's RGB888 in v9.
static uint8_t dispBuf[LCD_H_RES * LVGL_BUF_LINES * (LV_COLOR_DEPTH / 8)]
    __attribute__((aligned(4)));

// BoardConfig's swap/mirror flags -> Arduino_GFX rotation number.
static uint8_t displayRotationFromConfig() {
  bool gfxMirrorX = !DISPLAY_MIRROR_X;
  bool gfxMirrorY = DISPLAY_MIRROR_Y;
  if (!DISPLAY_SWAP_XY && !gfxMirrorX && !gfxMirrorY) return 0;
  if (DISPLAY_SWAP_XY && gfxMirrorX && !gfxMirrorY) return 1;
  if (!DISPLAY_SWAP_XY && gfxMirrorX && gfxMirrorY) return 2;
  if (DISPLAY_SWAP_XY && !gfxMirrorX && gfxMirrorY) return 3;
  if (!DISPLAY_SWAP_XY && gfxMirrorX && !gfxMirrorY) return 4;
  if (DISPLAY_SWAP_XY && gfxMirrorX && gfxMirrorY) return 5;
  if (!DISPLAY_SWAP_XY && !gfxMirrorX && gfxMirrorY) return 6;
  return 7;
}

// The expander has no read-modify-write, so shadow the output byte.
static void setExpanderPin(uint8_t pin, bool level) {
  static uint8_t state = 0xFF;
  if (level) state |= (1 << pin);
  else state &= ~(1 << pin);
  ioExpander.digitalWriteByte(state);
}

static void lcdResetByExpander() {
  if (!ioExpander.begin(PCF8574_ADDR, &Wire)) {
    Serial.println("PCF8574 init failed");
    return;
  }

  ioExpander.digitalWriteByte(0xFF);
  delay(20);
  setExpanderPin(PCF8574_PIN_LCD_RST, false);
  delay(120);
  setExpanderPin(PCF8574_PIN_LCD_RST, true);
  delay(120);
}

static void lvglFlush(lv_display_t *drv, const lv_area_t *area, uint8_t *px_map) {
  uint32_t width = lv_area_get_width(area);
  uint32_t height = lv_area_get_height(area);

  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, width, height);

  lv_display_flush_ready(drv);
}

static void lvglTouchRead(lv_indev_t *, lv_indev_data_t *data) {
  static int16_t lastX = 0;
  static int16_t lastY = 0;

  if (touch.touched()) {
    CST_TS_Point point = touch.getPoint();
    lastX = constrain(point.x, 0, (int16_t)gfx->width() - 1);
    lastY = constrain(point.y, 0, (int16_t)gfx->height() - 1);
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }

  data->point.x = lastX;
  data->point.y = lastY;
}

static void lvglBegin() {
  lv_init();

  // v9 has no LV_TICK_CUSTOM, the tick comes from here instead.
  lv_tick_set_cb([]() -> uint32_t { return millis(); });
  lv_delay_set_cb([](uint32_t ms) { delay(ms); });

  disp = lv_display_create(gfx->width(), gfx->height());
  lv_display_set_flush_cb(disp, lvglFlush);
  lv_display_set_buffers(disp, dispBuf, nullptr, sizeof(dispBuf),
                         LV_DISPLAY_RENDER_MODE_PARTIAL);

  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, lvglTouchRead);
  lv_indev_set_display(indev, disp);
}

void display_init() {
  if (disp != nullptr) return;

  Wire.begin(NEXTION_IIC_SDA_PIN, NEXTION_IIC_SCL_PIN, 400000);
  lcdResetByExpander();

  gfx->begin(LCD_SPI_FREQ);
  gfx->setRotation(displayRotationFromConfig());
  gfx->fillScreen(RGB565_BLACK);

  display_set_backlight(true);

  if (!touch.begin(&Wire, CST826_ADDR)) {
    Serial.println("CST826 init warning");
  }

  lvglBegin();
}

uint32_t display_task() {
  return lv_timer_handler();
}

void display_set_backlight(bool on) {
  pinMode(LCD_BL_PIN, OUTPUT);
  digitalWrite(LCD_BL_PIN, on ? HIGH : LOW);
}

lv_display_t *display_get() {
  return disp;
}
