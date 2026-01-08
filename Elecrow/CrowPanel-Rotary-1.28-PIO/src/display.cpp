#include "display.h"
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <Wire.h>
#include "CST816D.h"

// Pin definitions for 1.28 inch board
#define TP_I2C_SDA_PIN 6
#define TP_I2C_SCL_PIN 7
#define I2C_SDA_PIN 38
#define I2C_SCL_PIN 39

#define TP_INT 5
#define TP_RST 13
#define SCREEN_BACKLIGHT_PIN 46

// Backlight PWM
static const int pwmFreq = 5000;
static const int pwmChannel = 0;
static const int pwmResolution = 8;

// LovyanGFX Display Configuration for GC9A01
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_GC9A01 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
public:
    LGFX(void) {
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 80000000;
            cfg.freq_read = 20000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;
            cfg.pin_sclk = 10;
            cfg.pin_mosi = 11;
            cfg.pin_miso = -1;
            cfg.pin_dc = 3;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 9;
            cfg.pin_rst = 14;
            cfg.pin_busy = -1;
            cfg.memory_width = 240;
            cfg.memory_height = 240;
            cfg.panel_width = 240;
            cfg.panel_height = 240;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = false;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};

static LGFX gfx;
static CST816D touch(TP_I2C_SDA_PIN, TP_I2C_SCL_PIN, TP_RST, TP_INT);

// LVGL Draw Buffers
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

// Display flush callback for LVGL
static void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    if (gfx.getStartCount() > 0) {
        gfx.endWrite();
    }
    gfx.pushImageDMA(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1, (lgfx::rgb565_t *)&color_p->full);
    lv_disp_flush_ready(disp);
}

// Touch panel read callback for LVGL
static void touchpad_read_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    bool touched;
    uint8_t gesture;
    uint16_t touchX, touchY;

    touched = touch.getTouch(&touchX, &touchY, &gesture);

    if (!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    }
}

void display_init() {
    Serial.println("[Display] Init I2C...");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    Serial.println("[Display] Init touch...");
    touch.begin();

    // Initialize display
    Serial.println("[Display] GFX init...");
    gfx.init();
    gfx.initDMA();
    gfx.startWrite();
    gfx.setColor(0, 0, 0);
    gfx.fillScreen(TFT_BLACK);

    // Initialize LVGL
    Serial.println("[Display] LVGL init...");
    lv_init();

    // Allocate full-screen draw buffers in PSRAM
    size_t buf_size = sizeof(lv_color_t) * SCREEN_WIDTH * SCREEN_HEIGHT;
    Serial.printf("[Display] Alloc buffers: %d bytes\n", buf_size);

    buf1 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);
    buf2 = (lv_color_t *)heap_caps_malloc(buf_size, MALLOC_CAP_SPIRAM);

    if (!buf1) {
        Serial.println("[Display] Buffer 1 alloc FAILED!");
        return;
    }
    if (!buf2) {
        Serial.println("[Display] Buffer 2 alloc FAILED!");
        return;
    }
    Serial.println("[Display] Buffers OK");

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, SCREEN_WIDTH * SCREEN_HEIGHT);

    // Initialize display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize touch input driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read_cb;
    lv_indev_drv_register(&indev_drv);

    Serial.println("[Display] Driver init done");
}

void display_start() {
    Serial.println("[Display] Starting backlight...");
    delay(200);

    // Initialize backlight PWM
    ledcSetup(pwmChannel, pwmFreq, pwmResolution);
    ledcAttachPin(SCREEN_BACKLIGHT_PIN, pwmChannel);
    ledcWrite(pwmChannel, 128);  // 50% brightness

    Serial.println("[Display] Ready!");
}

void display_update() {
    lv_timer_handler();
}

void display_set_brightness(uint8_t brightness) {
    ledcWrite(pwmChannel, brightness);
}
