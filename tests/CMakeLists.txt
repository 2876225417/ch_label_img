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

macro(create_test_group_target group_name property_name)
    get_property(test_targets GLOBAL PROPERTY ${property_name})
    if (test_targets)
        add_custom_target(${group_name} ALL COMMENT "Building all ${group_name}")
        add_dependencies(${group_name} ${test_targets})
        pretty_message(STATUS "${group_name} targets: ${test_targets}")
    else()
        add_custom_target(${group_name} ALL COMMENT "No ${group_name} found")
    endif()
endmacro()

add_subdirectory(common)

if (BUILD_UNIT_TESTS)
    add_subdirectory(unit)
    create_test_group_target(unit_tests ALL_UNIT_TEST_TARGETS)
endif()

if (BUILD_PERFORMANCE_TESTS)
    add_subdirectory(performance)
    create_test_group_target(performance_tests ALL_PERF_TEST_TARGETS)
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
            COMMENT "Generating coverage report..."
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
