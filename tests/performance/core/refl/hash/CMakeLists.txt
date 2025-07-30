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

add_test(
    NAME Performance.HashBenchmarks
    COMMAND hash_benchmarks --benchmark_color=True
)

set_tests_properties(
    Performance.HashBenchmarks
    PROPERTIES LABELS "performance"
)

add_test_with_perf_stat(hash_benchmarks)

add_profiling_target_with_perf_record(hash_benchmarks)

add_test_with_valgrind(hash_benchmarks)