cmake_minimum_required(VERSION 3.16)


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

add_compile_definitions("PROJECT_NAME=\"${PROJECT_NAME}\"")
add_compile_definitions("PROJECT_VERSION=\"${PROJECT_VERSION}\"")
add_compile_definitions("PROJECT_VERSION_MAJOR=\"${PROJECT_VERSION_MAJOR}\"")
add_compile_definitions("PROJECT_VERSION_MINOR=\"${PROJECT_VERSION_MINOR}\"")
add_compile_definitions("PROJECT_VERSION_PATCH=\"${PROJECT_VERSION_PATCH}\"")
