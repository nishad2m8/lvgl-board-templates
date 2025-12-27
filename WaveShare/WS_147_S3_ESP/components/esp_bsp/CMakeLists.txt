file(GLOB_RECURSE SRC_FILES "*.c")

idf_component_register(SRCS ${SRC_FILES}
                    INCLUDE_DIRS "."
                    REQUIRES "esp_lcd" "driver" "esp_adc" "esp_lcd_jd9853" "esp_lcd_touch_axs5106"  "fatfs" "nvs_flash" "lwip" "esp_wifi")
