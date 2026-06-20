include(${CMAKE_CURRENT_LIST_DIR}/ui/CMakeLists.txt)

idf_component_register(
    SRCS "main.c" "board_init.c" ${UI_SRCS}
    INCLUDE_DIRS "." ${UI_INCLUDE_DIRS}
    EMBED_FILES ${UI_EMBED_FILES}
)
