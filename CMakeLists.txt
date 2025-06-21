cmake_minimum_required(VERSION 3.16)

project(ch_label_img LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type" FORCE)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(PrettyPrint)
include(ModuleInfo)


find_package(Qt6 REQUIRED COMPONENTS Widgets)

if(Qt6_FOUND)

endif()


pretty_message(INFO  "Configuring project: ${PROJECT_NAME} v${PROJECT_VERSION}")
pretty_message(VINFO "======================Compiler Info============================")
pretty_message(VINFO "  CMAKE_GENERATOR:                ${CMAKE_GENERATOR}")
pretty_message(VINFO "  CMAKE_CXX_STANDARD:             ${CMAKE_CXX_STANDARD}")
pretty_message(VINFO "  CMAKE_CXX_COMPILER:             ${CMAKE_CXX_COMPILER}")
pretty_message(VINFO "  CMAKE_CXX_COMPILER_VERSION:     ${CMAKE_CXX_COMPILER_VERSION}")
pretty_message(VINFO "  CMAKE_CXX_BYTE_ORDER:           ${CMAKE_CXX_BYTE_ORDER}")
pretty_message(VINFO "  CMAKE_CXX_COMPILER_ABI:         ${CMAKE_CXX_COMPILER_ABI}")
pretty_message(VINFO "  CMAKE_BUILD_TYPE:               ${CMAKE_BUILD_TYPE}")
pretty_message(VINFO "  CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
pretty_message(VINFO "===============================================================")

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

show_all_modules_summary()