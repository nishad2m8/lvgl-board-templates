#include "epd_display.h"
#include <Arduino.h>
#include "epd_driver/EPD_3in97.h"
#include "epd_driver/DEV_Config.h"

// Internal constants
#define EPD_BW_ROW_BYTES      ((EPD_WIDTH + 7) / 8)
#define EPD_FRAMEBUFFER_SIZE  (EPD_BW_ROW_BYTES * EPD_HEIGHT)
#define LVGL_BUFFER_LINES     64
#define LVGL_DRAW_BUF_SIZE    (EPD_WIDTH * LVGL_BUFFER_LINES * sizeof(lv_color16_t))

// File-scope state
static uint8_t *epd_framebuffer      = NULL;
static uint8_t *lvgl_draw_buf        = NULL;
static uint8_t *epd_partial_bw_buf   = NULL;
static size_t   epd_partial_bw_buf_size = 0;

static bool     need_full_refresh    = true;
static lv_area_t flush_area;
static bool     flush_area_valid     = false;
static bool     suppress_epd_refresh = false;

// Internal helpers
static uint32_t lvgl_tick_cb(void)
{
    return (uint32_t)millis();
}

static inline void epd_set_bw_pixel(int32_t x, int32_t y, bool white)
{
    uint32_t byte_index = y * EPD_BW_ROW_BYTES + (x >> 3);
    uint8_t bit = 7 - (x & 0x7);
    if (white) epd_framebuffer[byte_index] |= (1u << bit);
    else       epd_framebuffer[byte_index] &= ~(1u << bit);
}

// LVGL flush callback
static void epd_refresh_area(const lv_area_t *area);

static void epd_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    if (!epd_framebuffer) {
        lv_display_flush_ready(disp);
        return;
    }

    int32_t area_w = lv_area_get_width(area);
    int32_t area_h = lv_area_get_height(area);
    int32_t line_stride = area_w * sizeof(lv_color16_t);

    for (int32_t y = 0; y < area_h; y++) {
        int32_t abs_y = area->y1 + y;
        if (abs_y < 0 || abs_y >= EPD_HEIGHT) continue;

        const uint16_t *src_row = (const uint16_t *)(px_map + y * line_stride);

        for (int32_t x = 0; x < area_w; x++) {
            int32_t abs_x = area->x1 + x;
            if (abs_x < 0 || abs_x >= EPD_WIDTH) continue;

            uint16_t raw = src_row[x];
            uint8_t r5 = (raw >> 11) & 0x1F;
            uint8_t g6 = (raw >> 5) & 0x3F;
            uint8_t b5 = (raw >> 0) & 0x1F;
            uint8_t r = (r5 << 3) | (r5 >> 2);
            uint8_t g = (g6 << 2) | (g6 >> 4);
            uint8_t b = (b5 << 3) | (b5 >> 2);
            uint8_t lum = (uint16_t)(r * 77 + g * 150 + b * 29) >> 8;
            epd_set_bw_pixel(abs_x, abs_y, lum >= 160);
        }
    }

    // Accumulate dirty area across flush chunks
    if (!flush_area_valid) {
        flush_area = *area;
        flush_area_valid = true;
    } else {
        if (area->x1 < flush_area.x1) flush_area.x1 = area->x1;
        if (area->y1 < flush_area.y1) flush_area.y1 = area->y1;
        if (area->x2 > flush_area.x2) flush_area.x2 = area->x2;
        if (area->y2 > flush_area.y2) flush_area.y2 = area->y2;
    }

    if (lv_display_flush_is_last(disp)) {
        if (!suppress_epd_refresh) {
            if (need_full_refresh) {
                need_full_refresh = false;
                EPD_3IN97_Display_Base(epd_framebuffer);
            } else {
                epd_refresh_area(&flush_area);
            }
        }
        flush_area_valid = false;
    }

    lv_display_flush_ready(disp);
}

// Area-based partial refresh (used by flush callback)
static void epd_refresh_area(const lv_area_t *area)
{
    if (!area) {
        EPD_3IN97_Init();
        EPD_3IN97_Display_Base(epd_framebuffer);
        return;
    }

    int32_t x1 = LV_CLAMP(area->x1, 0, EPD_WIDTH - 1);
    int32_t y1 = LV_CLAMP(area->y1, 0, EPD_HEIGHT - 1);
    int32_t x2 = LV_CLAMP(area->x2, 0, EPD_WIDTH - 1);
    int32_t y2 = LV_CLAMP(area->y2, 0, EPD_HEIGHT - 1);

    if (x1 == 0 && y1 == 0 && x2 >= EPD_WIDTH - 8 && y2 == EPD_HEIGHT - 1) {
        EPD_3IN97_Init();
        EPD_3IN97_Display_Base(epd_framebuffer);
        return;
    }

    int32_t aligned_x1 = (x1 / 8) * 8;
    int32_t aligned_x2 = ((x2 + 7) / 8) * 8 - 1;
    if (aligned_x2 >= EPD_WIDTH) aligned_x2 = EPD_WIDTH - 1;

    uint32_t width = aligned_x2 - aligned_x1 + 1;
    uint32_t height = y2 - y1 + 1;
    uint32_t row_bytes = (width + 7) / 8;
    uint32_t buf_size = row_bytes * height;

    if (!epd_partial_bw_buf || epd_partial_bw_buf_size < buf_size) {
        if (epd_partial_bw_buf) free(epd_partial_bw_buf);
        epd_partial_bw_buf = (uint8_t *)ps_malloc(buf_size);
        epd_partial_bw_buf_size = buf_size;
    }
    if (!epd_partial_bw_buf) {
        EPD_3IN97_Init();
        EPD_3IN97_Display_Base(epd_framebuffer);
        return;
    }

    // Y-inversion: Init uses 0x11=0x01 (Y-decrement)
    int32_t ram_y_hi = (EPD_HEIGHT - 1) - y1;
    int32_t ram_y_lo = (EPD_HEIGHT - 1) - y2;

    uint8_t *dst = epd_partial_bw_buf;
    for (int32_t y = y1; y <= y2; y++) {
        const uint8_t *src = &epd_framebuffer[y * EPD_BW_ROW_BYTES + (aligned_x1 >> 3)];
        memcpy(dst, src, row_bytes);
        dst += row_bytes;
    }

    EPD_3IN97_Display_Partial(epd_partial_bw_buf, aligned_x1, ram_y_lo, aligned_x2 + 1, ram_y_hi + 1);
}

// Public API
void epd_hardware_init(void)
{
    Serial.println("[HW] Initializing e-Paper GPIO/SPI..."); Serial.flush();
    DEV_Module_Init();

    Serial.println("[HW] Initializing e-Paper display..."); Serial.flush();
    EPD_3IN97_Init();
    EPD_3IN97_Clear();

    Serial.println("[HW] e-Paper ready.");
}

void epd_lvgl_setup(void)
{
    lv_init();
    lv_tick_set_cb(lvgl_tick_cb);

    epd_framebuffer = (uint8_t *)ps_malloc(EPD_FRAMEBUFFER_SIZE);
    lvgl_draw_buf   = (uint8_t *)ps_malloc(LVGL_DRAW_BUF_SIZE);

    if (!epd_framebuffer || !lvgl_draw_buf) {
        Serial.println("[LVGL] FATAL: Failed to allocate PSRAM buffers!");
        while (1) delay(1000);
    }

    memset(epd_framebuffer, 0xFF, EPD_FRAMEBUFFER_SIZE);
    memset(lvgl_draw_buf, 0xFF, LVGL_DRAW_BUF_SIZE);

    lv_display_t *disp = lv_display_create(EPD_WIDTH, EPD_HEIGHT);
    lv_display_set_default(disp);
    lv_display_set_flush_cb(disp, epd_flush_cb);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(disp, lvgl_draw_buf, NULL,
                           LVGL_DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    Serial.println("[LVGL] Display created: 800x480 RGB565, PARTIAL render mode");
}

void epd_set_suppress_refresh(bool suppress)
{
    suppress_epd_refresh = suppress;
}

bool epd_needs_full_refresh(void)
{
    return need_full_refresh;
}

void epd_refresh_region(const EpdRegion &region)
{
    int32_t x1 = (region.x / 8) * 8;
    int32_t x2 = ((region.x + region.w + 7) / 8) * 8 - 1;
    if (x2 >= EPD_WIDTH) x2 = EPD_WIDTH - 1;
    int32_t y1 = region.y;
    int32_t y2 = region.y + region.h - 1;
    if (y2 >= EPD_HEIGHT) y2 = EPD_HEIGHT - 1;

    uint32_t row_bytes = (x2 - x1 + 1) / 8;
    uint32_t height = y2 - y1 + 1;
    uint32_t buf_size = row_bytes * height;

    if (!epd_partial_bw_buf || epd_partial_bw_buf_size < buf_size) {
        if (epd_partial_bw_buf) free(epd_partial_bw_buf);
        epd_partial_bw_buf = (uint8_t *)ps_malloc(buf_size);
        epd_partial_bw_buf_size = buf_size;
    }
    if (!epd_partial_bw_buf) return;

    // Y-inversion: Init uses 0x11=0x01 (Y-decrement)
    int32_t ram_y_hi = (EPD_HEIGHT - 1) - y1;
    int32_t ram_y_lo = (EPD_HEIGHT - 1) - y2;

    uint8_t *dst = epd_partial_bw_buf;
    for (int32_t y = y1; y <= y2; y++) {
        memcpy(dst, &epd_framebuffer[y * EPD_BW_ROW_BYTES + (x1 >> 3)], row_bytes);
        dst += row_bytes;
    }

    Serial.printf("[PARTIAL] lvgl y1=%d y2=%d -> ram_y %d..%d\n",
                  y1, y2, ram_y_lo, ram_y_hi);
    EPD_3IN97_Display_Partial(epd_partial_bw_buf, x1, ram_y_lo, x2 + 1, ram_y_hi + 1);
}
