cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/widget)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/window)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/stacked_page)


file(GLOB_RECURSE PROJECT_HEADERS
    ${CMAKE_SOURCE_DIR}/include
    "*.h"
    "*.hpp"
)

add_executable(label_img
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${PROJECT_HEADERS}
)

target_include_directories(label_img
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(label_img
    PRIVATE
    widget
    window
    page
)