cmake_minimum_required(VERSION 3.16)


file(GLOB WIDGET_HEADERS
    "${CMAKE_SOURCE_DIR}/include/widget/*.h"
    "${CMAKE_SOURCE_DIR}/include/widget/*.hpp"
)

file(GLOB WIDGET_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

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
)

