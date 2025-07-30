#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <numeric>
namespace test::common::fixtures {
struct SampleData {
    static auto get_short_strings()
    -> const std::vector<std::string>& {
        static const std::vector<std::string> data = {
            "a",    "ab",    "abc",
            "test", "hello", "world",
            "hash", "perf"
        };
        return data;
    }

    static auto get_medium_strings()
    -> const std::vector<std::string>& {
        static const std::vector<std::string> data = {
            "this_is_a_medium_string",
            "another_medium_length_string",
            "performance_testing_string",
            "hash_algorithm_benchmark",
            "compile_time_hash_function",
            "runtime_hash_performance",
            "memory_efficient_hashing",
            "collision_resistant_hash"
        };
        return data;
    }

    static auto get_long_strings()
    -> const std::vector<std::string>& {
        static const std::vector<std::string> data = {
            "this_is_a_very_long_string_for_testing_hash_performance_with_larger_inputs_that_might_stress_the_algorithm",
            "another_extremely_long_string_that_contains_various_characters_and_patterns_to_test_hash_distribution_quality",
            "yet_another_long_string_with_repeating_patterns_repeating_patterns_repeating_patterns_for_collision_testing",
            "a_string_with_numbers_123456789_and_special_characters_!@#$%^&*()_+-=[]{}|;':\",./<>?`~_end"
        };
        return data;
    }

    static auto get_integers() 
    -> const std::vector<uint64_t>& {
        static const std::vector<uint64_t> data = {
            0, 1, 42, 255, 256, 65535, 65536,
            0xDEADBEEF, 0xCAFEBABE, 0x1234567890ABCDEF,
            std::numeric_limits<uint64_t>::max()
        };
        return data;
    }

    static auto get_edge_cases()
    -> const std::vector<std::string>& {
                static const std::vector<std::string> data = {
            "",                               // empty
            "\0",                             // null
            "\n\t\r\v\f",                     // blank
            "aaaaaaaaaaaaaaaa",               // repeated 
            "012345678901234567890123456789", // number series
            "\x01\x02\x03\x04\x05",           // binary
        };
        return data;
    }
};

struct PerfConfig {
    static constexpr size_t WARMUP_ITERATIONS          = 1000;
    static constexpr size_t MEASUREMENT_ITERATIONS     = 10000;
    static constexpr size_t STRESS_TEST_ITERATIONS     = 100000;

    static constexpr double SHORT_STRING_THREHOLD_US   = 1.0;
    static constexpr double MEDIUM_STRING_THRESHOLD_US = 5.0;
    static constexpr double LONG_STRING_THRESHOLD_US   = 20.0;
};




} // namespace test::common::fixtures