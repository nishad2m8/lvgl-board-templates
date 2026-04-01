#include "e1002_display.h"

#include <lvgl.h>
#include <TFT_eSPI.h>

/*Set to your screen resolution and rotation*/
#define TFT_HOR_RES   800
#define TFT_VER_RES   480

#define EPAPER_BLACK_UINT  0x000000
#define EPAPER_WHITE_UINT  0xFFFFFF
#define EPAPER_RED_UINT    0xFF0000
#define EPAPER_GREEN_UINT  0x00FF00
#define EPAPER_BLUE_UINT   0x0000FF
#define EPAPER_YELLOW_UINT 0xFFFF00

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
static uint8_t draw_buf[DRAW_BUF_SIZE];

EPaper epaper;

#if LV_USE_LOG != 0
void arduino_print( lv_log_level_t level, const char * buf )
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

// Convert RGB888 color to 6-color e-paper index
static uint8_t rgb888_to_epaper_6color(uint8_t r, uint8_t g, uint8_t b)
{
    // White: High brightness
    if (r > 200 && g > 200 && b > 200) {
        return TFT_WHITE;  // 0x0
    }
    
    // Black: Low brightness
    if (r < 50 && g < 50 && b < 50) {
        return TFT_BLACK;  // 0xF
    }
    
    // Red: Red component dominates
    if (r > g && r > b && r > 100) {
        return TFT_RED;    // 0x6
    }
    
    // Green: Green component dominates
    if (g > r && g > b && g > 100) {
        return TFT_GREEN;  // 0x2
    }
    
    // Blue: Blue component dominates
    if (b > r && b > g && b > 100) {
        return TFT_BLUE;   // 0xD
    }
    
    // Yellow: High red+green, low blue
    if (r > 100 && g > 100 && b < 100) {
        return TFT_YELLOW; // 0xB
    }
    
    // Gray area determination
    uint8_t avg = (r + g + b) / 3;
    if (avg > 128) {
        return TFT_WHITE;  // Light gray -> White
    } else {
        return TFT_BLACK;  // Dark gray -> Black
    }
}

/* LVGL calls it when a rendered image needs to copied to the display*/
static void e1002_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
    e1002_driver_t *drv = (e1002_driver_t *)lv_display_get_driver_data(disp);
    EPaper *epd = drv->epd;
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            lv_color_t lv_color = ((lv_color_t*)px_map)[y * w + x];
            uint8_t r = lv_color.red;
            uint8_t g = lv_color.green;
            uint8_t b = lv_color.blue;
            uint32_t color_hex = (r << 16) | (g << 8) | b;
            
            uint8_t epaper_clr_index;

            // --- 新增的智能处理逻辑 ---
            // 检查这个像素是否已经是我们支持的6种基本色之一
            // 如果是，直接使用，不做任何转换。这可以完美保留您抖动过的背景。
            switch(color_hex) {
                case EPAPER_BLACK_UINT:
                    epaper_clr_index = TFT_BLACK;
                    break;
                case EPAPER_WHITE_UINT:
                    epaper_clr_index = TFT_WHITE;
                    break;
                case EPAPER_RED_UINT:
                    epaper_clr_index = TFT_RED;
                    break;
                case EPAPER_GREEN_UINT:
                    epaper_clr_index = TFT_GREEN;
                    break;
                case EPAPER_BLUE_UINT:
                    epaper_clr_index = TFT_BLUE;
                    break;
                case EPAPER_YELLOW_UINT:
                    epaper_clr_index = TFT_YELLOW;
                    break;
                default:
                    // 如果不是6种基本色之一，我们假设它是文字的抗锯齿像素
                    // 这时再调用颜色转换函数，将其强制转换为最接近的纯色
                    epaper_clr_index = rgb888_to_epaper_6color(r, g, b);
                    break;
            }
            // --- 逻辑结束 ---

            epd->drawPixel(area->x1 + x, area->y1 + y, epaper_clr_index);
        }
    }

    if (lv_display_flush_is_last(disp)) {
        drv->flush_scheduled = true;
        drv->flush_scheduled_time = millis();
    }

    /*Call it to tell LVGL you are ready*/
    lv_display_flush_ready(disp);
}

/*Read the touchpad*/
static void device_touchpad_read( lv_indev_t * indev, lv_indev_data_t * data )
{
}

/*use Arduinos millis() as tick source*/
static uint32_t get_arduino_tick(void)
{
    return millis();
}

static void resolution_changed_event_cb(lv_event_t * e)
{
    lv_display_t *disp = (lv_display_t *)lv_event_get_target(e);
    e1002_driver_t *drv =  (e1002_driver_t *)lv_display_get_driver_data(disp);
    EPaper *epd = drv->epd;
    int32_t hor_res = lv_display_get_horizontal_resolution(disp);
    int32_t ver_res = lv_display_get_vertical_resolution(disp);
    lv_display_rotation_t rot = lv_display_get_rotation(disp);

    /* handle rotation */
    switch(rot) {
        case LV_DISPLAY_ROTATION_0:
            epd->setRotation(0);   /* Portrait orientation */
            break;
        case LV_DISPLAY_ROTATION_90:
            epd->setRotation(1);   /* Landscape orientation */
            break;
        case LV_DISPLAY_ROTATION_180:
            epd->setRotation(2);   /* Portrait orientation, flipped */
            break;
        case LV_DISPLAY_ROTATION_270:
            epd->setRotation(3);   /* Landscape orientation, flipped */
            break;
    }
}

void e1002_display_init(e1002_driver_t *drv)
{
    drv->epd = &epaper;
    drv->flush_scheduled = false;
    drv->flush_scheduled_time = 0;

    drv->epd->begin();

    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(get_arduino_tick);

    /* register print function for debugging */
#if LV_USE_LOG != 0
    lv_log_register_print_cb( arduino_print );
#endif

    lv_display_t *disp;

    /*Else create a display yourself*/
    disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
    lv_display_set_driver_data(disp, (void *)drv);
    lv_display_set_flush_cb(disp, e1002_disp_flush);
    //lv_display_add_event_cb(disp, resolution_changed_event_cb, LV_EVENT_RESOLUTION_CHANGED, NULL);
    lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /*Initialize the (dummy) input device driver*/
    // lv_indev_t * indev = lv_indev_create();
    // lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    // lv_indev_set_read_cb(indev, device_touchpad_read);

}

void e1002_display_refresh(e1002_driver_t *drv)
{
    drv->epd->update();
    drv->flush_scheduled = false;
}

void e1002_display_schedule_refresh(e1002_driver_t *drv)
{
    drv->flush_scheduled = true;
    drv->flush_scheduled_time = millis();
}

bool e1002_display_should_refresh(e1002_driver_t *drv)
{
    return drv->flush_scheduled && (millis() - drv->flush_scheduled_time > 100);
}
