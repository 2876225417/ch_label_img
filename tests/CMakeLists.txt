cmake_minimum_required(VERSION 3.16)

option(BUILD_UNIT_TESTS        "Build unit tests"        ON)
option(BUILD_INTEGRATION_TESTS "Build integration tests" ON)
option(BUILD_PERFORMANCE_TESTS "Build performance tests" ON)
option(ENABLE_COVERAGE         "Enable code coverage"    OFF)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG        v1.17.0
)
set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
set(BUILD_GMOCK            OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST            ON  CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if (BUILD_PERFORMANCE_TESTS)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_USE_BUNDLED_GTEST  OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_TESTING     OFF CACHE BOOL "" FORCE)


    FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark
        GIT_TAG v1.9.4
    )
    FetchContent_MakeAvailable(benchmark)
endif()

add_subdirectory(common)

if (BUILD_UNIT_TESTS)
    add_subdirectory(unit)
endif()

if (BUILD_INTEGRATION_TESTS)
    add_subdirectory(integration)
endif()

if (BUILD_PERFORMANCE_TESTS)
    add_subdirectory(performance)
endif()

if (BUILD_REGRESSION_TESTS)
    add_subdirectory(regression)
endif()

add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS unit_tests integration_tests

)

pretty_message(DEBUG "BUILD_TESTS=ON. Build with tests.")
pretty_message(VINFO_BANNER "Tests Info" "=" ${BANNER_WIDTH})
pretty_message_kv(VINFO "gtest_force_shared_crt" "${gtest_force_shared_crt}")
pretty_message_kv(VINFO "BUILD_GMOCK"            "${BUILD_GMOCK}")
pretty_message_kv(VINFO "BUILD_GTEST"            "${BUILD_GTEST}")
pretty_message(VINFO_LINE "=" ${BANNER_WIDTH})
pretty_message(STATUS "")
