
# test all hash 
add_executable(hash_tests
    test_algorithms.cpp
    test_compile_time.cpp
    test_quality.cpp
)

target_link_libraries(hash_tests
    PRIVATE
    test_common
    gtest_main
)

target_include_directories(hash_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

gtest_discover_tests(hash_tests
    PROPERTIES
        LABLES "unit;core;refl;hash;fast"
        TIMEOUT 60
)

set_property(GLOBAL APPEND PROPERTY ALL_UNIT_TEST_TARGETS hash_tests)


# test compile time
add_executable(hash_compile_time_tests
    test_compile_time.cpp
)

target_link_libraries(hash_compile_time_tests
    PRIVATE
    test_common
    gtest_main
)

target_include_directories(hash_compile_time_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_options(hash_compile_time_tests
    PRIVATE
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-O1>
)

gtest_discover_tests(hash_compile_time_tests
    PROPERTIES
        LABLES "unit;core;refl;hash;compile_time"
        TIMEOUT 60
)

set_property(GLOBAL APPEND PROPERTY ALL_UNIT_TEST_TARGETS hash_compile_time_tests)

# test performance
add_executable(hash_performance_tests
    test_performance.cpp
)

target_link_libraries(hash_performance_tests
    PRIVATE
    test_common
    gtest_main
)

if (BUILD_PERFORMANCE_TESTS AND TARGET benchmark::benchmark)
    target_link_libraries(hash_performance_tests
        PRIVATE
        benchmark::benchmark
    )

    target_compile_definitions(hash_performance_tests
        PRIVATE
        BUILD_PERFORMANCE_TESTS
    )
endif()

target_include_directories(hash_performance_tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests/common
)

gtest_discover_tests(hash_performance_tests
    PROPERTIES
        LABLES "performance;core;refl;hash;slow"
        TIMEOUT 300
)

set_property(GLOBAL APPEND PROPERTY ALL_PERF_TEST_TARGETS hash_performance_tests)

if (ENABLE_COVERAGE)
    foreach(target hash_tests hash_compile_time_tests hash_performance_tests)
        target_compile_options(${target} PRIVATE --coverage)
        target_link_options(${target} PRIVATE --coverage)
    endforeach()
endif()

if (BUILD_PERFORMANCE_TESTS AND TARGET benchmark::benchmark)
    add_executable(hash_benchmarks
        benchmark_hash.cpp
    )

    target_link_libraries(hash_benchmarks
        PRIVATE
        test_common
        benchmark::benchmark_main
    )

    target_include_directories(hash_benchmarks
        PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests/common
    )

    target_compile_definitions(hash_benchmarks 
        PRIVATE
        BUILD_PERFORMANCE_TESTS
    )

    set_property(GLOBAL APPEND PROPERTY ALL_PERF_TEST_TARGETS hash_benchmarks)
endif()