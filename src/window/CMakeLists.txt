cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE WINDOW_HEADERS 
    "${CMAKE_SOURCE_DIR}/include/window/*.h"
    "${CMAKE_SOURCE_DIR}/include/window/*.hpp"
)

file(GLOB_RECURSE WINDOW_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

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
