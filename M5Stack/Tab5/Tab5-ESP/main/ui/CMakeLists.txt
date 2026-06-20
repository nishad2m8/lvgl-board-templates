set(UI_SOURCE_PATTERNS
    "${CMAKE_CURRENT_LIST_DIR}/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/*.cc"
    "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/*.S"
)

set(UI_HEADER_PATTERNS
    "${CMAKE_CURRENT_LIST_DIR}/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/*.hh"
    "${CMAKE_CURRENT_LIST_DIR}/*.hpp"
)

set(UI_ASSET_PATTERNS
    "${CMAKE_CURRENT_LIST_DIR}/*.bin"
    "${CMAKE_CURRENT_LIST_DIR}/*.bmp"
    "${CMAKE_CURRENT_LIST_DIR}/*.gif"
    "${CMAKE_CURRENT_LIST_DIR}/*.jpeg"
    "${CMAKE_CURRENT_LIST_DIR}/*.jpg"
    "${CMAKE_CURRENT_LIST_DIR}/*.otf"
    "${CMAKE_CURRENT_LIST_DIR}/*.png"
    "${CMAKE_CURRENT_LIST_DIR}/*.raw"
    "${CMAKE_CURRENT_LIST_DIR}/*.ttf"
)

if(NOT CMAKE_SCRIPT_MODE_FILE)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${UI_SOURCE_PATTERNS}
        ${UI_HEADER_PATTERNS}
        ${UI_ASSET_PATTERNS}
    )
endif()

file(GLOB_RECURSE UI_SRCS ${UI_SOURCE_PATTERNS})
file(GLOB_RECURSE UI_HEADERS ${UI_HEADER_PATTERNS})
file(GLOB_RECURSE UI_EMBED_FILES ${UI_ASSET_PATTERNS})

set(_ui_include_dirs "${CMAKE_CURRENT_LIST_DIR}")
foreach(ui_header ${UI_HEADERS})
    get_filename_component(ui_header_dir "${ui_header}" DIRECTORY)
    list(APPEND _ui_include_dirs "${ui_header_dir}")
endforeach()
list(REMOVE_DUPLICATES _ui_include_dirs)

set(UI_SRCS ${UI_SRCS} PARENT_SCOPE)
set(UI_INCLUDE_DIRS ${_ui_include_dirs} PARENT_SCOPE)
set(UI_EMBED_FILES ${UI_EMBED_FILES} PARENT_SCOPE)
