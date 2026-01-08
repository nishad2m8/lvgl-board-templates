#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_arc_indicator;
extern const lv_img_dsc_t img_arc_main;
extern const lv_img_dsc_t img_img_glow;
extern const lv_img_dsc_t img_img_needle;
extern const lv_img_dsc_t img_seat_logo;
extern const lv_img_dsc_t img_img_battery;
extern const lv_img_dsc_t img_pressure_arc_main;
extern const lv_img_dsc_t img_image_oil;
extern const lv_img_dsc_t img_temp_arc_main;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[9];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/