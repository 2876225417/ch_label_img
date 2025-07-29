add_subdirectory(hash)
add_subdirectory(meta)
add_subdirectory(type)

add_executable(refl_core_tests
    test_reflection_core.cpp
)

target_link_libraries(refl_core_tests
    PRIVATE
    test_common
    gtest_main
)

target_include_directories(refl_core_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(refl_core_tests
    PROPERTIES
        LABLES "unit;core;refl"
        TIMEOUT 30
)

add_dependencies(unit_tests refl_core_tests)

if (ENABLE_COVERAGE)
    target_compile_options(refl_core_tests PRIVATE --coverage)
    target_link_options(refl_core_tests PRIVATE --coverage)
endif()
