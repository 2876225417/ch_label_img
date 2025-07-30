// ---------------Reflection.Hash.Performance--------------- //
//         
//     Description: 
//          Test performance for computing hash
//  
//     Algorithms: 
//        1. FNV-1a
//        2. DJB2
//        3. MURMUR3
//        4. CRC32
//
//
//
// -------------------------------------------------------- //


#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include "benchmark_utils.hpp"
#include "sample_data.hpp"

#ifdef BUILD_PERFORMANCE_TESTS
#include <benchmark/benchmark.h>
#endif

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <core/refl/detail/hash/hash_algorithms.hpp>

using namespace labelimg::core::refl::hash::algorithms;
using namespace test::common::fixtures;
using namespace test::common::utils::benchmark;

class HashPerformanceTest: public ::testing::Test {
protected:
    void SetUp() override {
        if (std::getenv("SKIP_PERF_TEST")) {
            GTEST_SKIP() << "Performance tests skipped via SKIP_PERF_TEST";
        }
    }
};

TEST_F(HashPerformanceTest, ShorStringPerformance) {
    const auto& test_strings = SampleData::get_short_strings();
    
    auto hash_algo_perf_for = [&]<StringHashAlgo Algorithm>() {
        using algo = typename AlgoSelector<Algorithm>::type;
        auto stats = benchmark_function(algo::name, [&]() {
            for (const auto& str: test_strings) {
                volatile auto hash = algo::compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(algo::name) + " - Short Strings(compile-time)");
        EXPECT_LT(
            stats.avg_time,
            PerfConfig::SHORT_STRING_THREHOLD_US * test_strings.size()
        );
    };

    auto runtime_hash_algo_perf_for = [&](StringHashAlgo algorithm) {
        RuntimeHashComputer runtime_hasher(algorithm);

        auto stats = benchmark_function(runtime_hasher.name(), [&]() {
            for (const auto& str: test_strings) {
                volatile auto hash = runtime_hasher.compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(runtime_hasher.name()) + " - Short Strings(runtime)");
    };

    // TODO(ppqwqqq): Table style output for comparison
    hash_algo_perf_for.template operator()<StringHashAlgo::fnv1a>();
    hash_algo_perf_for.template operator()<StringHashAlgo::djb2>();
    hash_algo_perf_for.template operator()<StringHashAlgo::murmur3>();
    hash_algo_perf_for.template operator()<StringHashAlgo::crc32>();

    runtime_hash_algo_perf_for(StringHashAlgo::fnv1a);
    runtime_hash_algo_perf_for(StringHashAlgo::djb2);
    runtime_hash_algo_perf_for(StringHashAlgo::murmur3);
    runtime_hash_algo_perf_for(StringHashAlgo::crc32);
}

TEST_F(HashPerformanceTest, MediumStringPerformance) {
    const auto& test_strings = SampleData::get_medium_strings();
    
    auto hash_algo_perf_for = [&]<StringHashAlgo Algorithm>() {
        using algo = typename AlgoSelector<Algorithm>::type;
        auto stats = benchmark_function(algo::name, [&]() {
            for (const auto& str: test_strings) {
                volatile auto hash = algo::compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(algo::name) + " - Medium Strings(compile-time)");
        EXPECT_LT(
            stats.avg_time,
            PerfConfig::MEDIUM_STRING_THRESHOLD_US * test_strings.size()
        );
    };


    auto runtime_hash_algo_perf_for = [&](StringHashAlgo algorithm) {
        RuntimeHashComputer runtime_hasher(algorithm);

        auto stats = benchmark_function(runtime_hasher.name(), [&]() {
            for (const auto& str: test_strings) {
                volatile auto hash = runtime_hasher.compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(runtime_hasher.name()) + " - Medium Strings(runtime)");
    };

    hash_algo_perf_for.template operator()<StringHashAlgo::fnv1a>();
    hash_algo_perf_for.template operator()<StringHashAlgo::djb2>();
    hash_algo_perf_for.template operator()<StringHashAlgo::murmur3>();
    hash_algo_perf_for.template operator()<StringHashAlgo::crc32>();

    runtime_hash_algo_perf_for(StringHashAlgo::fnv1a);
    runtime_hash_algo_perf_for(StringHashAlgo::djb2);
    runtime_hash_algo_perf_for(StringHashAlgo::murmur3);
    runtime_hash_algo_perf_for(StringHashAlgo::crc32);
}

TEST_F(HashPerformanceTest, LongStringPerformance) {
    const auto& test_strings = SampleData::get_long_strings();
    
    auto hash_algo_perf_for = [&]<StringHashAlgo Algorithm>() {
        using algo = typename AlgoSelector<Algorithm>::type;
        auto stats = benchmark_function(algo::name, [&]() {
            for (const auto& str: test_strings) {
                volatile auto hash = algo::compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(algo::name) + " - Long Strings(compile-time)");
        EXPECT_LT(
            stats.avg_time,
            PerfConfig::LONG_STRING_THRESHOLD_US * test_strings.size()
        );
    };

    auto runtime_hash_algo_perf_for = [&](StringHashAlgo algorithm) {
        RuntimeHashComputer runtime_hasher(algorithm);

        auto stats = benchmark_function(runtime_hasher.name(), [&]() {
            for (const auto& str: test_strings) {
                volatile auto hash = runtime_hasher.compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(runtime_hasher.name()) + " - Long Strings(runtime)");
    };

    hash_algo_perf_for.template operator()<StringHashAlgo::fnv1a>();
    hash_algo_perf_for.template operator()<StringHashAlgo::djb2>();
    hash_algo_perf_for.template operator()<StringHashAlgo::murmur3>();
    hash_algo_perf_for.template operator()<StringHashAlgo::crc32>();

    runtime_hash_algo_perf_for(StringHashAlgo::fnv1a);
    runtime_hash_algo_perf_for(StringHashAlgo::djb2);
    runtime_hash_algo_perf_for(StringHashAlgo::murmur3);
    runtime_hash_algo_perf_for(StringHashAlgo::crc32);
}

TEST_F(HashPerformanceTest, StressTestWithRandomData) {
    auto random_strings = DataGenerator::generate_strings(1000, 32);
    
    auto hash_algo_perf_for = [&]<StringHashAlgo Algorithm>() {
        using algo = typename AlgoSelector<Algorithm>::type;
        auto stats = benchmark_function(algo::name, [&]() {
            for (const auto& str: random_strings) {
                volatile auto hash = algo::compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(algo::name) + " - Random Strings(compile-time)");
        EXPECT_LT(
            stats.avg_time,
            PerfConfig::LONG_STRING_THRESHOLD_US * random_strings.size()
        );
    };

    auto runtime_hash_algo_perf_for = [&](StringHashAlgo algorithm) {
        RuntimeHashComputer runtime_hasher(algorithm);

        auto stats = benchmark_function(runtime_hasher.name(), [&]() {
            for (const auto& str: random_strings) {
                volatile auto hash = runtime_hasher.compute(str);
                (void)hash;
            }
        }, PerfConfig::MEASUREMENT_ITERATIONS);

        stats.print(std::string(runtime_hasher.name()) + " - Random Strings(runtime)");
    };

    hash_algo_perf_for.template operator()<StringHashAlgo::fnv1a>();
    hash_algo_perf_for.template operator()<StringHashAlgo::djb2>();
    hash_algo_perf_for.template operator()<StringHashAlgo::murmur3>();
    hash_algo_perf_for.template operator()<StringHashAlgo::crc32>();

    runtime_hash_algo_perf_for(StringHashAlgo::fnv1a);
    runtime_hash_algo_perf_for(StringHashAlgo::djb2);
    runtime_hash_algo_perf_for(StringHashAlgo::murmur3);
    runtime_hash_algo_perf_for(StringHashAlgo::crc32);
}

TEST_F(HashPerformanceTest, DistributionVsPerformanceTradeOff) {
    auto similar_strings = DataGenerator::generate_similar_strings(10000);

    auto hash_algo_perf_for = [&]<StringHashAlgo Algorithm>() {
        using algo = typename AlgoSelector<Algorithm>::type;
        
        Timer timer;
        std::unordered_set<size_t> hashes;

        for (const auto& str: similar_strings) {
            auto hash = algo::compute(str);
            hashes.insert(hash);
        }

        double hash_time = timer.elapsed_ms();

        double collision_rate = 1.0 - (static_cast<double>(hashes.size()) / similar_strings.size());

        std::cout << "=== Distribution vs Performance ===" << '\n';
        std::cout << "Hash algorithm: " << algo::name << '\n';
        std::cout << "Hash time: " << hash_time << " ms" << '\n';
        std::cout << "Unique hashes: " << hashes.size() << " / " << similar_strings.size() << '\n';
        std::cout << "Collision rate: " << (collision_rate * 100) << "%" << '\n';

        EXPECT_LT(hash_time, 100.0);
        EXPECT_LT(collision_rate, 0.01);
    };

    hash_algo_perf_for.template operator()<StringHashAlgo::fnv1a>();
    hash_algo_perf_for.template operator()<StringHashAlgo::djb2>();
    hash_algo_perf_for.template operator()<StringHashAlgo::murmur3>();
    hash_algo_perf_for.template operator()<StringHashAlgo::crc32>();
}

TEST_F(HashPerformanceTest, CompileTimeHashTest) {
    constexpr auto compile_time_hash1 = compute_with_algorithm<StringHashAlgo::fnv1a>("compile_time_test");
    constexpr auto compile_time_hash2 = compute_with_algorithm<StringHashAlgo::fnv1a>("different_string");

    RuntimeHashComputer runtime_hasher(StringHashAlgo::fnv1a);
    auto runtime_hash1 = runtime_hasher.compute("compile_time_test");
    auto runtime_hash2 = runtime_hasher.compute("different_string");
    
    EXPECT_EQ(compile_time_hash1, runtime_hash1);
    EXPECT_EQ(compile_time_hash2, runtime_hash2);
    EXPECT_NE(compile_time_hash1, compile_time_hash2);
    EXPECT_NE(runtime_hash1, runtime_hash2);

    std::cout << "=== Compile-time Hash Test ===" << std::endl;
    std::cout << "Compile-time hash 1: 0x" << std::hex << compile_time_hash1 << '\n';
    std::cout << "Runtime hash 1:      0x" << std::hex << runtime_hash1 << '\n';
    std::cout << "Hashes match: " << (compile_time_hash1 == runtime_hash1 ? "YES" : "NO") << '\n';
    

}
