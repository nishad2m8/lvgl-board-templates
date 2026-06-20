# M5Tab5 User Demo

User demo source code of [M5Tab5](https://docs.m5stack.com/en/products/sku/k145).

## Project Layout

```text
.
├── CMakeLists.txt
├── sdkconfig
├── sdkconfig.defaults
├── components/
├── main/
│   ├── CMakeLists.txt
│   ├── board_init.c
│   ├── board_init.h
│   ├── main.c
│   └── ui/
│       ├── CMakeLists.txt
│       ├── ui.c
│       ├── screens.c
│       ├── styles.c
│       └── ...
├── managed_components/
├── dependencies.lock
├── partitions.csv
└── README.md
```

`main/ui/CMakeLists.txt` scans the whole `main/ui` tree for `.c/.cc/.cpp/.S` files and also watches common image/font assets such as `.png`, `.jpg`, `.bmp`, `.bin`, `.ttf`, and `.otf`. New UI files can be added there without editing `main/CMakeLists.txt`.

## IDF Build

#### Tool Chains

[ESP-IDF v5.5.3](https://docs.espressif.com/projects/esp-idf/en/v5.5.3/esp32p4/index.html)

#### Build

```bash
idf.py build
```

#### Flash

```bash
idf.py flash
```

## Acknowledgments

This project references the following open-source libraries and resources:

- https://github.com/lvgl/lvgl
- https://www.heroui.com
- https://github.com/alexreinert/piVCCU/blob/master/kernel/rtc-rx8130.c
- https://components.espressif.com/components/espressif/esp_cam_sensor
- https://components.espressif.com/components/espressif/esp_ipa
- https://components.espressif.com/components/espressif/esp_sccb_intf
- https://components.espressif.com/components/espressif/esp_video
- https://components.espressif.com/components/espressif/esp_lvgl_port
- https://github.com/jarzebski/Arduino-INA226
- https://github.com/boschsensortec/BMI270_SensorAPI
