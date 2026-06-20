# UI component (EEZ Studio generated). Auto-discovers sources and subdirs
# so you can drop in images/, fonts/, screens/, etc. without editing this file.

file(GLOB_RECURSE UI_SRCS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Collect every directory that contains a header so includes "just work"
# regardless of how assets are organized under components/ui/.
file(GLOB_RECURSE UI_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

set(UI_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}")
foreach(_hdr ${UI_HEADERS})
    get_filename_component(_dir "${_hdr}" DIRECTORY)
    list(APPEND UI_INCLUDE_DIRS "${_dir}")
endforeach()
list(REMOVE_DUPLICATES UI_INCLUDE_DIRS)

idf_component_register(
    SRCS        ${UI_SRCS}
    INCLUDE_DIRS ${UI_INCLUDE_DIRS}
    REQUIRES    lvgl__lvgl)
