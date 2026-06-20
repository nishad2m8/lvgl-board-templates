#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_chicken;
extern const lv_img_dsc_t img_dragon;
extern const lv_img_dsc_t img_duck;
extern const lv_img_dsc_t img_quail;
extern const lv_img_dsc_t img_egg_chicken;
extern const lv_img_dsc_t img_egg_dragon;
extern const lv_img_dsc_t img_egg_duck;
extern const lv_img_dsc_t img_egg_quail;
extern const lv_img_dsc_t img_heater;
extern const lv_img_dsc_t img_humidifier;
extern const lv_img_dsc_t img_indicator;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[11];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/