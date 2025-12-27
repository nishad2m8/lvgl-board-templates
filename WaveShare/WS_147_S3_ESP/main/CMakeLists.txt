idf_component_register(SRCS "main.c" "lv_fs_port.c"
                    INCLUDE_DIRS "."
                    REQUIRES ui esp_bsp nvs_flash espressif__esp_lvgl_port)