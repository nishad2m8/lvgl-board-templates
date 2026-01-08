#include "display.h"
#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include <Adafruit_CST8XX.h>
#include "PCF8574.h"

// I2C Pins
#define I2C_SDA_PIN 38
#define I2C_SCL_PIN 39

// PCF8574 I/O Expander
PCF8574 pcf8574(0x21);

// Backlight PWM
#define SCREEN_BACKLIGHT_PIN 6
static const int pwmFreq = 5000;
static const int pwmChannel = 0;
static const int pwmResolution = 8;

// Touch Panel
#define I2C_TOUCH_ADDR 0x15
static Adafruit_CST8XX tsPanel = Adafruit_CST8XX();

// LVGL Draw Buffers (LVGL 9 uses uint8_t*)
static uint8_t *buf1 = NULL;
static uint8_t *buf2 = NULL;

// LVGL Display and Input Device handles
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_indev = NULL;

// RGB Panel Bus - ST7701 Display Controller
static Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
    16 /* CS */, 2 /* SCK */, 1 /* SDA */,
    40 /* DE */, 7 /* VSYNC */, 15 /* HSYNC */, 41 /* PCLK */,
    46 /* R0 */, 3 /* R1 */, 8 /* R2 */, 18 /* R3 */, 17 /* R4 */,
    14 /* G0 */, 13 /* G1 */, 12 /* G2 */, 11 /* G3 */, 10 /* G4 */, 9 /* G5 */,
    5 /* B0 */, 45 /* B1 */, 48 /* B2 */, 47 /* B3 */, 21 /* B4 */
);

static Arduino_ST7701_RGBPanel *gfx = new Arduino_ST7701_RGBPanel(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */,
    false /* IPS */, 480 /* width */, 480 /* height */,
    st7701_type5_init_operations, sizeof(st7701_type5_init_operations),
    true /* BGR */,
    10 /* hsync_front_porch */, 4 /* hsync_pulse_width */, 20 /* hsync_back_porch */,
    10 /* vsync_front_porch */, 4 /* vsync_pulse_width */, 20 /* vsync_back_porch */
);

// Display flush callback for LVGL 9
static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    // Draw RGB565 bitmap (16-bit color)
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);

    lv_display_flush_ready(disp);
}

// Touch panel read callback for LVGL 9
static void touchpad_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    if (tsPanel.touched()) {
        CST_TS_Point p = tsPanel.getPoint(0);
        data->point.x = p.x;
        data->point.y = p.y - 20;  // Y offset from factory code
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

// LVGL tick callback for Arduino millis
static uint32_t my_tick_get_cb(void) {
    return millis();
}

void display_init() {
    Serial.println("[Display] Init I2C...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    // Initialize PCF8574 with pin modes (same as factory)
    pcf8574.pinMode(P0, OUTPUT);        // Touch RST
    pcf8574.pinMode(P2, OUTPUT);        // Touch INT
    pcf8574.pinMode(P3, OUTPUT);        // LCD power
    pcf8574.pinMode(P4, OUTPUT);        // LCD reset
    pcf8574.pinMode(P5, INPUT_PULLUP);  // Encoder switch

    Serial.println("[Display] Init PCF8574...");
    if (pcf8574.begin()) {
        Serial.println("[Display] PCF8574 OK");
    } else {
        Serial.println("[Display] PCF8574 FAILED!");
        return;
    }

    // LCD Power ON
    pcf8574.digitalWrite(P3, HIGH);
    delay(100);

    // LCD Reset sequence (same as factory)
    pcf8574.digitalWrite(P4, HIGH);
    delay(100);
    pcf8574.digitalWrite(P4, LOW);
    delay(120);
    pcf8574.digitalWrite(P4, HIGH);
    delay(120);

    // Touch panel reset (same as factory)
    pcf8574.digitalWrite(P0, HIGH);
    delay(100);
    pcf8574.digitalWrite(P0, LOW);
    delay(120);
    pcf8574.digitalWrite(P0, HIGH);
    delay(120);
    pcf8574.digitalWrite(P2, HIGH);
    delay(120);

    // Initialize display
    Serial.println("[Display] GFX begin...");
    gfx->begin();
    gfx->fillScreen(BLACK);

    // Initialize touch panel
    Serial.println("[Display] Touch init...");
    if (!tsPanel.begin(&Wire, I2C_TOUCH_ADDR)) {
        Serial.println("[Display] Touch FAILED!");
    } else {
        Serial.println("[Display] Touch OK");
    }

    // Initialize LVGL
    Serial.println("[Display] LVGL init...");
    lv_init();

    // Set tick callback for LVGL 9
    lv_tick_set_cb(my_tick_get_cb);

    // Allocate full-screen draw buffers in PSRAM
    // For RGB565 (16-bit): 2 bytes per pixel
    size_t buf_size = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint16_t);
    Serial.printf("[Display] Alloc buffers: %d bytes\n", buf_size);

    buf1 = (uint8_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
    buf2 = (uint8_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);

    if (!buf1 || !buf2) {
        Serial.println("[Display] Buffer alloc FAILED!");
        return;
    }
    Serial.println("[Display] Buffers OK");

    // Create LVGL display (LVGL 9 API)
    lvgl_disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(lvgl_disp, disp_flush_cb);
    lv_display_set_buffers(lvgl_disp, buf1, buf2, buf_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create touch input device (LVGL 9 API)
    lvgl_indev = lv_indev_create();
    lv_indev_set_type(lvgl_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(lvgl_indev, touchpad_read_cb);

    Serial.println("[Display] Driver init done");
}

// Call AFTER ui_init() - same sequence as factory
void display_start() {
    Serial.println("[Display] Starting backlight...");
    delay(200);

    // Initialize backlight PWM
    ledcSetup(pwmChannel, pwmFreq, pwmResolution);
    ledcAttachPin(SCREEN_BACKLIGHT_PIN, pwmChannel);
    ledcWrite(pwmChannel, 204);  // ~80% brightness like factory

    // LCD power stabilize (same as factory)
    pcf8574.digitalWrite(P3, LOW);

    Serial.println("[Display] Ready!");
}

void display_update() {
    lv_timer_handler();
}

void display_set_brightness(uint8_t brightness) {
    ledcWrite(pwmChannel, brightness);
}

// Read encoder switch from PCF8574
bool display_read_switch() {
    return pcf8574.digitalRead(P5, true) == LOW;
}
