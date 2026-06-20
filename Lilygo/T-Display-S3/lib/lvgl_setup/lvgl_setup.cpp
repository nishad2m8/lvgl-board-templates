#include "lvgl_setup.h"
#include "TFT_eSPI.h"

// TFT instance
TFT_eSPI tft = TFT_eSPI();

// Static variables for LVGL
static lv_display_t *disp;
static lv_color_t *buf1;
static lv_color_t *buf2;

// LVGL 9 flush callback
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)px_map, w * h, true);
    tft.endWrite();

    lv_display_flush_ready(disp);
}

#if LV_USE_LOG != 0
// Serial debugging callback
static void lvgl_log_cb(lv_log_level_t level, const char *buf)
{
    Serial.printf("[LVGL] ");
    Serial.print(buf);
    Serial.flush();
}
#endif

// Initialize LVGL and display
void lvgl_setup_init(uint8_t rotation)
{
    // Initialize serial for debugging
    if (!Serial) {
        Serial.begin(115200);
    }
    
    Serial.println("Initializing LVGL setup...");

    // Initialize LVGL
    lv_init();

    // Register logging callback if enabled
    #if LV_USE_LOG != 0
        lv_log_register_print_cb(lvgl_log_cb);
    #endif

    // Initialize TFT display
    tft.begin();
    tft.setRotation(rotation);
    
    // Calculate buffer size (1/10 of screen)
    uint32_t buf_size = SCREEN_WIDTH * SCREEN_HEIGHT / 10;
    
    // Allocate display buffers
    buf1 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * buf_size, MALLOC_CAP_DMA);
    buf2 = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * buf_size, MALLOC_CAP_DMA);
    
    if (!buf1 || !buf2) {
        Serial.println("Failed to allocate display buffers!");
        return;
    }

    // Create display with LVGL 9 API
    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    lv_display_set_buffers(disp, buf1, buf2, buf_size * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Create input device (if touch is available)
    // Note: You'll need to implement touch read callback if using touchscreen
    // lv_indev_t *indev = lv_indev_create();
    // lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    // lv_indev_set_read_cb(indev, touch_read_cb);

    Serial.println("LVGL setup complete!");
}

// Optional: Call this from a timer interrupt for LVGL tick
void lvgl_setup_tick_inc(void)
{
    lv_tick_inc(5);
}