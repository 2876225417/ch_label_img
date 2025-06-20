cmake_minimum_required(VERSION 3.16)

file(GLOB PAGE_HEADERS
    "${CMAKE_SOURCE_DIR}/include/page/*.h"
    "${CMAKE_SOURCE_DIR}/include/page/*.hpp"
)

file(GLOB PAGE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

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

