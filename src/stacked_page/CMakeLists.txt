cmake_minimum_required(VERSION 3.16)

module_begin("Stacked_page")

check_module_dependencies("Stacked_page"
    REQUIRES Qt6::Widgets
    OPTIONAL Qt6::Designer Qt6::UiTools
)

# =============== stacked_page configuration ===============
file(GLOB PAGE_HEADERS
    "${CMAKE_SOURCE_DIR}/include/stacked_page/*.h"
    "${CMAKE_SOURCE_DIR}/include/stacked_page/*.hpp"
)

file(GLOB PAGE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

module_show_files("Headers" "${PAGE_HEADERS}")
module_show_files("Sources" "${PAGE_SOURCES}")

add_library(page
    SHARED
    ${PAGE_HEADERS}
    ${PAGE_SOURCES}
)

target_include_directories(page
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(page
    PUBLIC
    Qt6::Widgets    
)

auto_enable_pch(page stacked_page)

set_target_properties(page PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "page"
)

# ==========================================================

module_target_info(page SHOW_PROPERTIES)
module_show_dependencies(page)
module_end("Stacked_page" page SHOW_SUMMARY)