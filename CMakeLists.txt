cmake_minimum_required(VERSION 3.16)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Release)


project(ch_label_img LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

file(GLOB_RECURSE HEADERS 
    ${CMAKE_SOURCE_DIR}/include
    "*.h"
)


add_executable(main 
    ${HEADERS}
    src/main.cpp
    src/widget/region_cropper.cpp
    src/window/mainwindow.cpp
)

target_include_directories(main
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)


target_link_libraries(main
    PRIVATE
    Qt6::Widgets
)

