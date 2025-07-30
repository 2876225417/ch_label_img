cmake_minimum_required(VERSION 3.16)

option(BUILD_UNIT_TESTS        "Build unit tests"        ON)
option(BUILD_INTEGRATION_TESTS "Build integration tests" OFF)
option(BUILD_PERFORMANCE_TESTS "Build performance tests" OFF)
option(BUILD_REGRESSION_TESTS  "Build regression tests"  OFF)
option(ENABLE_COVERAGE         "Enable code coverage"    OFF)

set_property(GLOBAL PROPERTY ALL_UNIT_TEST_TARGETS "")
set_property(GLOBAL PROPERTY ALL_PERF_TEST_TARGETS "")

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG        v1.17.0
)
set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
set(BUILD_GMOCK            ON  CACHE BOOL "" FORCE)
set(BUILD_GTEST            ON  CACHE BOOL "" FORCE)
set(INSTALL_GTEST          OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
find_package(GTest REQUIRED)

if (BUILD_PERFORMANCE_TESTS)
    find_package(benchmark QUIET)
    # 使用配置好的 googletest
    if (NOT benchmark_FOUND)
        FetchContent_Declare(
            benchmark
            GIT_REPOSITORY https://github.com/google/benchmark
            GIT_TAG v1.9.4
        )
        set(BENCHMARK_ENABLE_TESTING         OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_INSTALL         OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_ENABLE_DOXYGEN         OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_INSTALL_DOCS           OFF CACHE BOOL "" FORCE)
        set(BENCHMARK_DOWNLOAD_DEPENDENCIES  ON  CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(benchmark)
    endif()
endif()

# 覆盖率测试
if (ENABLE_COVERAGE)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -g -O0)
        add_link_options(--coverage)
    endif()
endif()



add_subdirectory(common)

if (BUILD_UNIT_TESTS)
    add_subdirectory(unit)
    
    get_property(unit_test_targets GLOBAL PROPERTY ALL_UNIT_TEST_TARGETS)
    if (unit_test_targets)
        add_custom_target(unit_tests
            ALL
            COMMENT "Building all unit tests"    
        )
        add_dependencies(unit_tests ${unit_test_targets})

        add_custom_target(fast_tests # exclude perf tests
            COMMAND ${CMAKE_CTEST_COMMAND} -L "fast" --output-on-failure
            DEPENDS ${unit_test_targets}
            COMMENT "Running fast unit tests"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        pretty_message(STATUS "unit test targets: ${unit_test_targets}")
    else()
        add_custom_target(unit_tests ALL
            COMMENT "No unit tests found"
        )
    endif()
endif()

if (BUILD_PERFORMANCE_TESTS)
    add_subdirectory(performance)

    get_property(perf_test_targets GLOBAL PROPERTY ALL_PERF_TEST_TARGETS)
    if (perf_test_targets)
        add_custom_target(performance_tests 
            ALL
            COMMENT "Building all performance tests"
        )
        add_dependencies(performance_tests ${perf_test_targets})

        add_custom_target(run_performance_tests
            COMMAND ${CMAKE_CTEST_COMMAND} -L "performance" --output-on-failure
            DEPENDS ${perf_test_targets}
            COMMENT "Running performance tests"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        add_custom_target(run_benchmarks
            COMMENT "Running all benchmark executable"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

        foreach (target ${perf_test_targets})
            add_custom_command(TARGET run_benchmarks POST_BUILD
                COMMAND $<TARGET_FILE:${target}> --benchmark_format=console
                COMMENT "Running ${target}"
            )
        endforeach()

        pretty_message(STATUS "performance test targets: ${perf_test_targets}")
    else()
        add_custom_target(performance_tests 
            ALL
            COMMENT "No performance tests found"
        )
    endif()
endif()

if (BUILD_INTEGRATION_TESTS)
    add_subdirectory(integration)
endif()

if (BUILD_REGRESSION_TESTS)
    add_subdirectory(regression)
endif()

set(all_test_targets ${unit_test_targets})
if (BUILD_PERFORMANCE_TESTS AND perf_test_targets)
    list(APPEND all_test_targets ${perf_test_targets})
endif()

if (all_test_targets)
    add_custom_target(all_tests
        ALL
        COMMENT "Building all tests"
    )
    add_dependencies(all_tests ${all_test_targets})
endif()

add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS ${all_test_targets}
    COMMENT "Running all tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(test_unit
    COMMAND ${CMAKE_CTEST_COMMAND} -L "unit" --output-on-failure
    DEPENDS ${unit_test_targets}
    COMMENT "Running unit tests"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

if (BUILD_PERFORMANCE_TESTS)
    add_custom_target(test_perf
        COMMAND ${CMAKE_CTEST_COMMAND} -L "performance" --output-on-failure
        DEPENDS ${perf_test_targets}
        COMMENT "Running performance tests"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# 覆盖率报告目标
if (ENABLE_COVERAGE)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if (LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            # 清理之前的覆盖率数据
            COMMAND ${LCOV_PATH} --directory . --zerocounters
            # 运行测试
            COMMAND ${CMAKE_CTEST_COMMAND} -L "unit" --output-on-failure
            # 收集覆盖率测试
            COMMAND ${LCOV_PATH} --directory . --capture --output-file
            # 过滤系统头文件和测试文件
            COMMAND ${LCOV_PATH} --remove coverage.info
                '/usr/*'
                '*/test/*'
                '*/build/*'
                '3rdparty/*'
                '/_deps/*'
                --output-file coverage_filtered.info
            # 生成 HTML 报告
            COMMAND ${GENHTML_PATH} coverage_filtered.info --output-directory coverage_html

            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report"
        )
    endif()
endif()

pretty_message(DEBUG "BUILD_TESTS=ON. Build with tests.")
pretty_message(VINFO_BANNER "Tests Info" "=" ${BANNER_WIDTH})
pretty_message_kv(VINFO "BUILD_UNIT_TESTS"        "${BUILD_UNIT_TESTS}")
pretty_message_kv(VINFO "BUILD_INTEGRATION_TESTS" "${BUILD_INTEGRATION_TESTS}")
pretty_message_kv(VINFO "BUILD_PERFORMANCE_TESTS" "${BUILD_PERFORMANCE_TESTS}")
pretty_message_kv(VINFO "ENABLE_COVERAGE"         "${ENABLE_COVERAGE}")
pretty_message_kv(VINFO "gtest_force_shared_crt" "${gtest_force_shared_crt}")
pretty_message_kv(VINFO "BUILD_GMOCK"            "${BUILD_GMOCK}")
pretty_message_kv(VINFO "BUILD_GTEST"            "${BUILD_GTEST}")
pretty_message(VINFO_LINE "=" ${BANNER_WIDTH})
pretty_message(STATUS "")
