#include "display.h"
#include "lv_conf.h"
#include "HWCDC.h"
#include <Wire.h>

extern HWCDC USBSerial;

#define LVGL_TICK_PERIOD_MS 2

// Display and bus objects
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    LCD_CS, LCD_SCLK, LCD_SDIO0, LCD_SDIO1, LCD_SDIO2, LCD_SDIO3);

Arduino_SH8601 *gfx = new Arduino_SH8601(
    bus, GFX_NOT_DEFINED, 0, LCD_WIDTH, LCD_HEIGHT);

// I2C bus for touch
std::shared_ptr<Arduino_IIC_DriveBus> IIC_Bus =
    std::make_shared<Arduino_HWIIC>(IIC_SDA, IIC_SCL, &Wire);

// Touch interrupt handler
void Arduino_IIC_Touch_Interrupt(void);

std::unique_ptr<Arduino_IIC> FT3168(new Arduino_FT3x68(IIC_Bus, FT3168_DEVICE_ADDRESS,
                                                       DRIVEBUS_DEFAULT_VALUE, TP_INT, Arduino_IIC_Touch_Interrupt));

void Arduino_IIC_Touch_Interrupt(void) {
    FT3168->IIC_Interrupt_Flag = true;
}

// IO Expander
Adafruit_XCA9554 expander;

// Screen dimensions
uint32_t screenWidth;
uint32_t screenHeight;

// LVGL draw buffer
static lv_disp_draw_buf_t draw_buf;

#if LV_USE_LOG != 0
static void lvgl_print_cb(const char *buf) {
    Serial.printf(buf);
    Serial.flush();
}
#endif

// Display flush callback
static void disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
    gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

    lv_disp_flush_ready(disp);
}

// Touchpad read callback
static int32_t lastX = 0;
static int32_t lastY = 0;
static bool touchActive = false;

static void touchpad_read_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    if (FT3168->IIC_Interrupt_Flag == true) {
        FT3168->IIC_Interrupt_Flag = false;

        int32_t touchPoints = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_FINGER_NUMBER);

        if (touchPoints > 0) {
            lastX = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_X);
            lastY = FT3168->IIC_Read_Device_Value(FT3168->Arduino_IIC_Touch::Value_Information::TOUCH_COORDINATE_Y);
            touchActive = true;
        } else {
            touchActive = false;
        }
    }

    data->point.x = lastX;
    data->point.y = lastY;
    data->state = touchActive ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

void lvgl_tick_callback(void *arg) {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

void display_init() {
    // Initialize I2C
    Wire.begin(IIC_SDA, IIC_SCL);

    // Initialize IO expander
    if (!expander.begin(0x20)) {
        Serial.println("Failed to find XCA9554 chip");
        while (1);
    }

    // Configure expander pins
    expander.pinMode(0, OUTPUT);
    expander.pinMode(1, OUTPUT);
    expander.pinMode(2, OUTPUT);
    expander.pinMode(6, OUTPUT);
    expander.digitalWrite(0, LOW);
    expander.digitalWrite(1, LOW);
    expander.digitalWrite(2, LOW);
    expander.digitalWrite(6, LOW);
    delay(20);
    expander.digitalWrite(0, HIGH);
    expander.digitalWrite(1, HIGH);
    expander.digitalWrite(2, HIGH);
    expander.digitalWrite(6, HIGH);

    // Initialize display
    gfx->begin();
    gfx->setBrightness(200);

    screenWidth = gfx->width();
    screenHeight = gfx->height();

    // Initialize LVGL
    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(lvgl_print_cb);
#endif

    // Allocate draw buffers
    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 4 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(screenWidth * screenHeight / 4 * sizeof(lv_color_t), MALLOC_CAP_DMA);

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 4);

    // Initialize display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.sw_rotate = 1;
    lv_disp_drv_register(&disp_drv);

    // Create LVGL tick timer
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_callback,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000);

    USBSerial.println("Display initialized");
}

void touch_init() {
    // Initialize touch controller
    while (FT3168->begin() == false) {
        USBSerial.println("FT3168 initialization fail");
        delay(2000);
    }
    USBSerial.println("FT3168 initialization successfully");

    // Set touch power mode
    FT3168->IIC_Write_Device_State(FT3168->Arduino_IIC_Touch::Device::TOUCH_POWER_MODE,
                                   FT3168->Arduino_IIC_Touch::Device_Mode::TOUCH_POWER_MONITOR);

    // Initialize input device driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read_cb;
    lv_indev_drv_register(&indev_drv);

    USBSerial.println("Touch initialized");
}
