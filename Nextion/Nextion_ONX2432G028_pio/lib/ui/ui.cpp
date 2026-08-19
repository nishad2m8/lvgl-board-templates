#include "ui.h"

// Label + slider. Stock lv_conf, so montserrat_14 is the only font we have.

static lv_obj_t *titleLabel = nullptr;
static lv_obj_t *valueLabel = nullptr;
static lv_obj_t *slider = nullptr;

static void slider_event_cb(lv_event_t *e) {
  lv_obj_t *target = (lv_obj_t *)lv_event_get_target(e);
  lv_label_set_text_fmt(valueLabel, "%d %%", (int)lv_slider_get_value(target));
}

void ui_init(void) {
  if (slider != nullptr) return;

  lv_obj_t *screen = lv_screen_active();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x101418), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

  titleLabel = lv_label_create(screen);
  lv_label_set_text(titleLabel, "Nextion ONX2432G028");
  lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xF2F5F7), LV_PART_MAIN);
  lv_obj_align(titleLabel, LV_ALIGN_CENTER, 0, -50);

  slider = lv_slider_create(screen);
  lv_obj_set_width(slider, 180);
  lv_slider_set_range(slider, 0, 100);
  lv_slider_set_value(slider, 50, LV_ANIM_OFF);
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, nullptr);

  valueLabel = lv_label_create(screen);
  lv_label_set_text_fmt(valueLabel, "%d %%", (int)lv_slider_get_value(slider));
  lv_obj_set_style_text_color(valueLabel, lv_color_hex(0x8AB4F8), LV_PART_MAIN);
  lv_obj_align_to(valueLabel, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
}

int32_t ui_get_slider_value(void) {
  return slider ? lv_slider_get_value(slider) : 0;
}

void ui_set_slider_value(int32_t value, bool animate) {
  if (slider == nullptr) return;
  lv_slider_set_value(slider, value, animate ? LV_ANIM_ON : LV_ANIM_OFF);
  lv_label_set_text_fmt(valueLabel, "%d %%", (int)lv_slider_get_value(slider));
}
