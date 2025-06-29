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

include(cmake/Config.cmake)

pretty_message(VINFO_BANNER "Configuring Project" "=" ${BANNER_WIDTH})
pretty_message_kv(VINFO "PROJECT_NAME"                   "${PROJECT_NAME}")
pretty_message_kv(VINFO "PROJECT_VERSION"                "${PROJECT_VERSION}")
pretty_message(VINFO_LINE "=" ${BANNER_WIDTH})
pretty_message(STATUS "")

pretty_message(VINFO_BANNER "Compiler Info" "=" ${BANNER_WIDTH})
pretty_message_kv(VINFO "CMAKE_GENERATOR"                "${CMAKE_GENERATOR}")
pretty_message_kv(VINFO "CMAKE_CXX_STANDARD"             "${CMAKE_CXX_STANDARD}")
pretty_message_kv(VINFO "CMAKE_CXX_COMPILER"             "${CMAKE_CXX_COMPILER}")
pretty_message_kv(VINFO "CMAKE_CXX_COMPILER_VERSION"     "${CMAKE_CXX_COMPILER_VERSION}")
pretty_message_kv(VINFO "CMAKE_CXX_BYTE_ORDER"           "${CMAKE_CXX_BYTE_ORDER}")
pretty_message_kv(VINFO "CMAKE_CXX_COMPILER_ABI"         "${CMAKE_CXX_COMPILER_ABI}")
pretty_message_kv(VINFO "CMAKE_BUILD_TYPE"               "${CMAKE_BUILD_TYPE}")
pretty_message_kv(VINFO "CMAKE_RUNTIME_OUTPUT_DIRECTORY" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
pretty_message(VINFO_LINE "=" ${BANNER_WIDTH})
pretty_message(STATUS "")
print_pretty_debug_info()
pretty_message(STATUS "")
print_project_info_debug_info()
pretty_message(STATUS "")

dependency_manager_init()

find_dependency(fmt REQUIRED)
find_dependency(magic_enum REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
if(Qt6_FOUND)
    show_qt_info()
    pretty_message(STATUS "")
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/src)

# pch 配置
pch_configure()
setup_project_pch()
show_pch_stats()
show_all_modules_summary()
