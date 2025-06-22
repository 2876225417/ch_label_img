cmake_minimum_required(VERSION 3.16)

module_begin("Qtils")
check_module_dependencies("Qtils"
    REQUIRES Qt6::Widgets
)

# =============== qtils configuration ===============
file(GLOB QTILS_HEADERS
    "${CMAKE_SOURCE_DIR}/include/qtils/*.h"
    "${CMAKE_SOURCE_DIR}/include/qtils/*.hpp"
)

file(GLOB QTILS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

module_show_files("Headers" "${QTILS_HEADERS}")
module_show_files("Sources" "${QTILS_SOURCES}")

add_library(qtils
    SHARED
    ${QTILS_HEADERS}
    ${QTILS_SOURCES}
)

target_include_directories(qtils
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(qtils
    PUBLIC
    Qt6::Widgets    
)

auto_enable_pch(qtils qtils)

set_target_properties(qtils PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "qtils"
)
# ===================================================





module_target_info(qtils SHOW_PROPERTIES)
module_show_dependencies(qtils)
module_end("Qtils" qtils SHOW_SUMMARY)