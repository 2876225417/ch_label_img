#pragma once


#include <chrono>
#include <cstdint>
#include <ios>
#include <limits>
#include <ratio>
#include <vector>
#include <string>
#include <random>
#include <functional>
#include <iostream>
#include <iomanip>
#include <algorithm>

#ifdef BUILD_PERFORMANCE_TESTS
#include <benchmark/benchmark.h>
#endif

namespace test::common::utils::benchmark {
class Timer {
public:
    Timer(): start_(std::chrono::high_resolution_clock::now()) { }

    void reset() { start_ = std::chrono::high_resolution_clock::now(); }

    [[nodiscard]]
    auto elapsed_ms() const -> double { 
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        return duration.count() / 1000.0;
    }

    [[nodiscard]]
    auto elapsed_us() const -> double {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        return static_cast<double>(duration.count());
    }
private:
    std::chrono::high_resolution_clock::time_point start_;
};

struct PerfStats {
    double min_time   = std::numeric_limits<double>::max();
    double max_time   = 0.0;
    double total_time = 0.0;
    double avg_time   = 0.0;
    size_t iterations = 0;

    void add_measurement(double time) {
        min_time   = std::min(min_time, time);
        max_time   = std::max(max_time, time);
        total_time += time;
        ++iterations;
        avg_time   = total_time / iterations;
    }

    void print(const std::string& name) const {
        std::cout << std::fixed     << std::setprecision(3);
        std::cout << "=== " << name << " Performance Stats ===" << '\n';
        std::cout << "Iterations: " << iterations << '\n';
        std::cout << "Min time: "   << min_time << '\n';
        std::cout << "Max time: "   << max_time << '\n';
        std::cout << "Avg time: "   << avg_time << '\n';
        std::cout << "Total time: " << total_time << '\n';
        std::cout << "Througput: "  << (iterations * 1000000.0 / total_time) << " ops/sec" << '\n';
    }
};

class DataGenerator {
public:
    static auto 
    generate_strings(size_t count, size_t length) 
    -> std::vector<std::string> {
        std::vector<std::string> result;
        result.reserve(count);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis('a', 'z');

        for (size_t i = 0; i < count; ++i) {
            std::string str;
            str.reserve(length);
            for (size_t j = 0; j < length; ++j) 
                str += static_cast<char>(dis(gen));
            result.push_back(std::move(str));
        }
        return result;
    }

    static auto
    generate_similar_strings(size_t count)
    -> std::vector<std::string> {
        std::vector<std::string> result;
        result.reserve(count);
        
        std::string prefix = "similar_string_";

        for (int i = 0; i < count; ++i)
            result.push_back(prefix + std::to_string(i));

        return result;
    }

    static auto
    generate_realistic_strings(size_t count) 
    -> std::vector<std::string> {
        std::vector<std::string> patterns = {
            "user_id_",
            "session_token_",
            "file_path_/home/user/documents",
            "email_address_user",
            "class_name_",
            "function_name_calculate_",
            "variable_name_temp_",
            "url_https://ppqwqqq.space/api/v1"
        };

        std::vector<std::string> result;
        result.reserve(count);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> pattern_dis(0, patterns.size() - 1);
        std::uniform_int_distribution<> num_dis(100000, 999999);

        for (size_t i = 0; i < count; ++i) {
            auto& pattern = patterns[pattern_dis(gen)];
            result.push_back(pattern + std::to_string(num_dis(gen)));
        }

        return result;
    }

    static auto generate_integers(size_t count) 
    -> std::vector<uint64_t> {
        std::vector<std::uint64_t> result;
        result.reserve(count);

        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;
        
        for (size_t i = 0; i < count; ++i) 
            result.push_back(dis(gen));

        return result;
    }

    
};

#define PERF_TEST(name) \
    void perf_test_##name(); \
    TEST(PerformanceTest, name) { \
        if (std::getenv("SKIP_PERF_TESTS")) { \
            GTEST_SKIP() << "Performance tests skipped"; \
        } \
    } \
    void perf_test_##name()

template <typename F>
auto 
benchmark_function(const std::string& name, F&& func, size_t iterations = 10000)
-> PerfStats {
    PerfStats stats;

    for (int i = 0; i < 100; ++i) 
        func();

    for (size_t i = 0; i < iterations; ++i) {
        Timer timer;
        func();
        stats.add_measurement(timer.elapsed_us());
    }
    return stats;
}

}  // namespace test::common::utils::benchmark