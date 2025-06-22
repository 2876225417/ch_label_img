cmake_minimum_required(VERSION 3.16)

module_begin("Window")

check_module_dependencies("Window"
    REQUIRES Qt6::Widgets
)

file(GLOB_RECURSE WINDOW_HEADERS 
    "${CMAKE_SOURCE_DIR}/include/window/*.h"
    "${CMAKE_SOURCE_DIR}/include/window/*.hpp"
)

file(GLOB_RECURSE WINDOW_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

module_show_files("Headers" "${WINDOW_HEADERS}")
module_show_files("Sources" "${WINDOW_SOURCES}")


add_library(window
    SHARED
    ${WINDOW_HEADERS}
    ${WINDOW_SOURCES}
)

target_include_directories(window
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(window
    PUBLIC
    Qt6::Widgets    
)

auto_enable_pch(window window)


set_target_properties(window PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "window"
)

module_target_info(window SHOW_PROPERTIES)
module_show_dependencies(window)
module_end("Window" window SHOW_SUMMARY)