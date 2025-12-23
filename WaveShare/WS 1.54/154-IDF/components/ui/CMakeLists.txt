file(GLOB_RECURSE ui_srcs ./*.c)
idf_component_register(
    SRCS ${ui_srcs}
    REQUIRES lvgl
    INCLUDE_DIRS ".")
