cmake_minimum_required(VERSION 3.16)

module_begin("Utils")

# =============== utils configuration ===============
file(GLOB UTILS_ONLY_HEADERS
    "${CMAKE_SOURCE_DIR}/include/utils/*.h"
    "${CMAKE_SOURCE_DIR}/include/utils/*.hpp"
)

module_show_files("Headers" "${UTILS_ONLY_HEADERS}")

add_library(utils INTERFACE ${UTILS_ONLY_HEADERS})

target_include_directories(utils
    INTERFACE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(utils
    INTERFACE
    proj_config
)

set_target_properties(utils PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "utils"
)
# ===================================================

module_target_info(utils SHOW_PROPERTIES)
module_show_dependencies(utils)
module_end("Utils" utils SHOW_SUMMARY)