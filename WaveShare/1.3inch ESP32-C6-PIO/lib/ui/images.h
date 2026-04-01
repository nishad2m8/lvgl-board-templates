#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_frame_00;
extern const lv_img_dsc_t img_frame_01;
extern const lv_img_dsc_t img_frame_02;
extern const lv_img_dsc_t img_frame_03;
extern const lv_img_dsc_t img_frame_04;
extern const lv_img_dsc_t img_frame_05;
extern const lv_img_dsc_t img_frame_06;
extern const lv_img_dsc_t img_frame_07;
extern const lv_img_dsc_t img_frame_08;
extern const lv_img_dsc_t img_frame_09;
extern const lv_img_dsc_t img_frame_10;
extern const lv_img_dsc_t img_frame_11;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[12];

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/