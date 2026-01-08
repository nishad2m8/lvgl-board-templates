#ifndef EEZ_LVGL_UI_STYLES_H
#define EEZ_LVGL_UI_STYLES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Style: label_reading
lv_style_t *get_style_label_reading_MAIN_DEFAULT();
void add_style_label_reading(lv_obj_t *obj);
void remove_style_label_reading(lv_obj_t *obj);



#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_STYLES_H*/