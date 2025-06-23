cmake_minimum_required(VERSION 3.16)

set(BANNER_WIDTH 70 CACHE STRING "Banner width affecting all pretty_message with banner or title")

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/core"
    "${CMAKE_CURRENT_LIST_DIR}/deps"
    "${CMAKE_CURRENT_LIST_DIR}/misc"
)

# 交叉编译
# include(mingw-w64-toolchain)

include(PrettyPrint)

# core
include(PCH)

# deps
include(DependencyManager)

# misc
include(ModuleInfo)

include(QtInfo)



pretty_message(INFO "CMake configuration loading done.")