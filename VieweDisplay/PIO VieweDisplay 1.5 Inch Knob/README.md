# VieweDisplay 1.5" Knob — Rotation & Brightness (💥 Experimental)

## Rotate Display
- Anti-tear OFF (default): edit `src/main.cpp:1` and, after LVGL init (before `ui_init()`), add:
  - `lv_disp_set_rotation(lv_disp_get_default(), LV_DISP_ROT_90);`
  - Options: `LV_DISP_ROT_NONE`, `LV_DISP_ROT_90`, `LV_DISP_ROT_180`, `LV_DISP_ROT_270`
- Anti-tear ON: edit `lib/lvgl_port_v8/lvgl_port_v8.h:1` and set:
  - `#define LVGL_PORT_AVOID_TEARING_MODE 1` (or 2 / 3)
  - `#define LVGL_PORT_ROTATION_DEGREE 90` (0 / 90 / 180 / 270)
  - Note: With avoid tearing enabled, `lv_disp_set_rotation()` is not supported; use `LVGL_PORT_ROTATION_DEGREE` instead.

## Adjust Brightness
- Edit `src/main.cpp:1`, inside `setup()` after `panel->begin()`:
  - `auto bl = panel->getBacklight();`
  - `if (bl) bl->setBrightness(80); // 0–100`
- On/off: `bl->on();` / `bl->off();`
