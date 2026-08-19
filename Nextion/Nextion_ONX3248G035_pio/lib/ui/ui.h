#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_init(void);
int32_t ui_get_slider_value(void);
void ui_set_slider_value(int32_t value, bool animate);

#ifdef __cplusplus
}
#endif
