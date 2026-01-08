#include "styles.h"
#include "images.h"
#include "fonts.h"

#include "ui.h"
#include "screens.h"

//
// Style: label_reading
//

void init_style_label_reading_MAIN_DEFAULT(lv_style_t *style) {
    lv_style_set_text_font(style, &ui_font_k2d_35);
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
};

lv_style_t *get_style_label_reading_MAIN_DEFAULT() {
    static lv_style_t *style;
    if (!style) {
        style = lv_mem_alloc(sizeof(lv_style_t));
        lv_style_init(style);
        init_style_label_reading_MAIN_DEFAULT(style);
    }
    return style;
};

void add_style_label_reading(lv_obj_t *obj) {
    (void)obj;
    lv_obj_add_style(obj, get_style_label_reading_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

void remove_style_label_reading(lv_obj_t *obj) {
    (void)obj;
    lv_obj_remove_style(obj, get_style_label_reading_MAIN_DEFAULT(), LV_PART_MAIN | LV_STATE_DEFAULT);
};

//
//
//

void add_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*AddStyleFunc)(lv_obj_t *obj);
    static const AddStyleFunc add_style_funcs[] = {
        add_style_label_reading,
    };
    add_style_funcs[styleIndex](obj);
}

void remove_style(lv_obj_t *obj, int32_t styleIndex) {
    typedef void (*RemoveStyleFunc)(lv_obj_t *obj);
    static const RemoveStyleFunc remove_style_funcs[] = {
        remove_style_label_reading,
    };
    remove_style_funcs[styleIndex](obj);
}

