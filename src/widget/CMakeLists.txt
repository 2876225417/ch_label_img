cmake_minimum_required(VERSION 3.16)

module_begin("Widget")

check_module_dependencies("Widget"
    REQUIRES Qt6::Widgets
    OPTIONAL Qt6::Designer Qt6::UiTools
)

module_feature("Widget" "CUSTOM_STYLES" ON "Enable custom widget styles")
module_feature("Widget" "ANIMATIONS"    ON "Enable widget animations")

# =============== widget configuration ===============
file(GLOB WIDGET_HEADERS
    "${CMAKE_SOURCE_DIR}/include/widget/*.h"
    "${CMAKE_SOURCE_DIR}/include/widget/*.hpp"
)

file(GLOB WIDGET_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

module_show_files("Headers" "${WIDGET_HEADERS}")
module_show_files("Sources" "${WIDGET_SOURCES}")

add_library(widget
    SHARED
    ${WIDGET_HEADERS}
    ${WIDGET_SOURCES}
)

target_include_directories(widget
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(widget
    PUBLIC
    Qt6::Widgets
    qtils
    core
)

if (Widget_ENABLE_CUSTOM_STYLES)
    pretty_message(VINFO "Widget: ENABLE_CUSTOM_STYLES")
    target_compile_definitions(widget PRIVATE Widget_ENABLE_CUSTOM_STYLES)
endif()

if (Widget_ENABLE_ANIMATIONS)
    pretty_message(VINFO "Widget: ENABLE_ANIMATIONS")
    target_compile_definitions(widget PRIVATE Widget_ENABLE_ANIMATIONS)
endif()

auto_enable_pch(widget widget)

set_target_properties(widget PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "widget"
)
# ====================================================

module_target_info(widget SHOW_PROPERTIES)
module_show_dependencies(widget)
module_end("Widget" widget SHOW_SUMMARY)