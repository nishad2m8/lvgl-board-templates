idf_component_register(
    SRCS "main.cpp"
    REQUIRES
        lvgl__lvgl
        epaper_driver_bsp
        board_power_bsp
        lvgl_port
        ui
    PRIV_REQUIRES
        esp_timer
        driver
    INCLUDE_DIRS "./")
