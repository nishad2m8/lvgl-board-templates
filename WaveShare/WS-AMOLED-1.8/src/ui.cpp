#include "ui.h"

static lv_obj_t *label;
static lv_obj_t *slider;
static lv_obj_t *value_label;

// Slider event callback
static void slider_event_cb(lv_event_t *e) {
    lv_obj_t *slider_obj = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider_obj);

    // Update value label
    lv_label_set_text_fmt(value_label, "Value: %d", value);
}

void ui_init() {
    // Create main container
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 320, 300);
    lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(cont, 20, 0);

    // Create title label
    label = lv_label_create(cont);
    lv_label_set_text(label, "Hello LVGL!");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);

    // Create slider
    slider = lv_slider_create(cont);
    lv_obj_set_width(slider, 200);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Create value label
    value_label = lv_label_create(cont);
    lv_label_set_text(value_label, "Value: 50");
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_18, 0);
}

int32_t ui_get_slider_value() {
    return lv_slider_get_value(slider);
}
