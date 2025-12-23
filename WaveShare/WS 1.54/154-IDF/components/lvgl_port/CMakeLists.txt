idf_component_register(
    SRCS "lvgl_port.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        lvgl__lvgl
        epaper_driver_bsp
        freertos
        esp_timer
    PRIV_REQUIRES
        main
)
