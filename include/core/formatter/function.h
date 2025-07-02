#ifndef FUNCTION_H
#define FUNCTION_H

#include <core/formatter/reflection.h>
#include <core/asm/hp_timer.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <ios>
#include <ostream>
#include <sstream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <array>
#include <string>


#if __cplusplus >= 202002L
#include <source_location>
#define HAS_SOURCE_LOCATION 1
#else
#define HAS_SOURCE_LOCATION 0
#endif

namespace labelimg::core::formatter::function {
using reflection::find_char;
using reflection::rfind_char;

using asm_::HighPrecisionTimer;

consteval auto 
extract_function_name(std::string_view pretty_function) 
-> std::string_view {
    size_t start = 0;
    
    // 跳过返回类型
    size_t space_pos = rfind_char(pretty_function, ' ', find_char(pretty_function, '('));
    if (space_pos != std::string_view::npos) start = space_pos + 1;

    // 处理成员函数的情况
    size_t scope_pos = rfind_char(pretty_function, ':', start);
    if (scope_pos != std::string_view::npos && scope_pos > start) start = scope_pos + 1;

    // 查找函数名的结束位置
    size_t end = find_char(pretty_function, '(', start);
    if (end == std::string_view::npos) end = pretty_function.size();

    return pretty_function.substr(start, end - start);
}

consteval auto 
extract_class_name(std::string_view pretty_function) 
-> std::string_view {
    size_t paren_pos = find_char(pretty_function, '(');
    if (paren_pos == std::string_view::npos) return "";

    size_t scope_pos = rfind_char(pretty_function, ':', paren_pos);
    if (scope_pos == std::string_view::npos || scope_pos == 0) return "";

    // 类名开始位置
    size_t start = rfind_char(pretty_function, ' ', scope_pos - 1);
    if (start == std::string_view::npos) start = 0;
    else start++;

    return pretty_function.substr(start, scope_pos - 1 - start);
}

consteval auto extract_namespace(std::string_view pretty_function) -> std::string_view {
    size_t paren_pos = find_char(pretty_function, '(');
    if (paren_pos == std::string_view::npos) return "";

    size_t first_scope = find_char(pretty_function, ':', 0);
    if (first_scope == std::string_view::npos || first_scope >= paren_pos) return "";

    size_t start = rfind_char(pretty_function, ' ', first_scope);
    if (start == std::string_view::npos) start = 0;
    else start++;

    return pretty_function.substr(start, first_scope - start);
}

consteval auto 
extract_filename(std::string_view file_path) 
-> std::string_view {
    size_t pos = rfind_char(file_path, '/');
    if (pos == std::string_view::npos) 
        pos = rfind_char(file_path, '\\');

    if (pos == std::string_view::npos) return file_path;
    return file_path.substr(pos + 1);
}

struct FunctionInfo {
    std::string_view function_name;
    std::string_view class_name;
    std::string_view namespace_name;
    std::string_view file_name;
    std::string_view full_signature;
    int line_number;
    
    [[nodiscard]] auto
    to_string() const -> std::string {
        std::string result;

        if (!namespace_name.empty()) {
            result += namespace_name;
            result += "::";
        }

        if (!class_name.empty()) {
            result += class_name;
            result += "::";
        }

        result += function_name;
        return result;
    }

    [[nodiscard]] auto
    to_detailed_string()
    const -> std::string {
        std::string result = to_string();
        result += " (";
        result += file_name;
        result += ":";
        result += std::to_string(line_number);
        result += ")";
        return result;
    }

    [[nodiscard]] auto
    to_full_string()
    const -> std::string {
        std::ostringstream oss;
        oss << "Function: "   << to_string() << "\n"
            << " File: "      << file_name   << ": " << line_number << "\n"
            << " Signature: " << full_signature;
        return oss.str();
    }

    [[nodiscard]] auto
    is_member_function()
    const -> bool {
        return !class_name.empty();
    }

    [[nodiscard]] auto
    has_namespace()
    const -> bool {
        return !namespace_name.empty();
    }
};

inline auto operator<<(std::ostream& os, const FunctionInfo& info) -> std::ostream& {
    os << info.to_string();
    return os;
}

struct DetailedFunctionInfo {
    const FunctionInfo& info;
    explicit DetailedFunctionInfo(const FunctionInfo& f)
        : info{f} { }
};

inline auto operator<<(std::ostream& os, const DetailedFunctionInfo& detailed) -> std::ostream& {
    os << detailed.info.to_detailed_string();
    return os;
}

struct FullFunctionInfo {
    const FunctionInfo& info;
    explicit FullFunctionInfo(const FunctionInfo& f) 
        : info{f} { }
};

inline auto operator>>(std::ostream& os, const FullFunctionInfo& full) -> std::ostream& {
    os << full.info.to_full_string();
    return os;
}

inline auto detailed(const FunctionInfo& info) -> DetailedFunctionInfo {
    return DetailedFunctionInfo{info};
}

inline auto full(const FunctionInfo& info) -> FullFunctionInfo {
    return FullFunctionInfo{info};
}



template <typename T = void>
class FunctionInfoExtractor {
public:
    template <auto PrettyFunction = __PRETTY_FUNCTION__>
    [[nodiscard]] static consteval auto
    get_info(const char* file = __FILE__, int line = __LINE__)
    -> FunctionInfo {
        constexpr std::string_view pretty_func = PrettyFunction;

        return FunctionInfo{
            .function_name = extract_function_name(pretty_func),
            .class_name    = extract_class_name(pretty_func),
            .namespace_name= extract_namespace(pretty_func),
            .file_name     = extract_filename(file),
            .full_signature= pretty_func,
            .line_number   = line
        };
    }

#if HAS_SOURCE_LOCATION
    [[nodiscard]] static consteval auto
    get_info_cpp20(const std::source_location& location = std::source_location::current()) 
    -> FunctionInfo {
        const auto func_name = location.function_name();
        
        return FunctionInfo{
            .function_name = extract_function_name(func_name),
            .class_name    = extract_class_name(func_name),
            .namespace_name= extract_namespace(func_name),
            .file_name     = extract_filename(location.file_name()),
            .full_signature= func_name,
            .line_number   = static_cast<int>(location.line())
        };
    }
#endif // HAS_SOURCE_LOCATION
};

// TODO(ppqwqqq): Replace macro to template
#define CURRENT_FUNCTION_INFO() \
    labelimg::core::formatter::function::FunctionInfoExtractor<>::get_info(__FILE__, __LINE__)

// 获取调用者函数信息(在被调用的函数中使用)
template <typename T = void>
[[nodiscard]] consteval auto
get_caller_info( const char* file = __FILE__
               , int line = __LINE__
               , const char* function = __PRETTY_FUNCTION__
               ) -> FunctionInfo {
    return FunctionInfo{
        .function_name = extract_function_name(function),
        .class_name    = extract_class_name(function),
        .namespace_name= extract_namespace(function),
        .file_name     = extract_filename(function),
        .full_signature= function,
        .line_number   = line
    };
}

#if HAS_SOURCE_LOCATION 
template <typename T = void>
[[nodiscard]] consteval auto
get_caller_info_cpp20(const std::source_location& location = std::source_location::current()) {
    return FunctionInfoExtractor<T>::get_info_cpp20(location);
}
#endif // HAS_SOURCE_LOCATION

template <typename ClassType>
class MemberFunctionInfoExtractor {
public:
    template <auto MemberPtr>
    [[nodiscard]] static consteval auto
    get_member_info() {
        // TODO: Specify the other infos 
        return FunctionInfo{}; 
    }
};

template <typename LambdaType>
class LambdaInfoExtractor {
public:
    [[nodiscard]] static consteval auto
    get_lambda_info() {
        return FunctionInfo{
            .function_name = "lambda",
            .class_name    = "",
            .namespace_name= "",
            .file_name     = "",
            .full_signature= __PRETTY_FUNCTION__,
            .line_number   = 0
        };
    }
};

template <typename FuncType>
struct FunctionSignatureAnalyzer;

template <typename R, typename... Args>
struct FunctionInfoExtractor<R(Args...)> {
    using RetType = R;
    using ArgTypes   = std::tuple<Args...>;
    static constexpr size_t argument_count = sizeof...(Args);

    template <size_t I>
    using ArgType = std::tuple_element_t<I, ArgTypes>;

    [[nodiscard]] static constexpr auto
    is_void_return() -> bool { return std::is_void_v<R>; }

    [[nodiscard]] static constexpr auto
    has_arguments() -> bool  { return argument_count > 0; }
};

template <typename R, typename C, typename... Args>
struct FunctionSignatureAnalyzer<R(C::*)(Args...)> {
    using RetType = R;
    using ClassType = C;
    using ArgTypes = std::tuple<Args...>;
    static constexpr size_t argument_count = sizeof...(Args);
    static constexpr bool is_member_function = true;

    template <size_t I>
    using ArgType = std::tuple_element_t<I, ArgTypes>;
};

template <typename T = void>
[[nodiscard]] consteval auto 
get_function_name(const char* pretty_function = __PRETTY_FUNCTION__) {
    return extract_function_name(pretty_function);
}

template <typename T = void>
[[nodiscard]] consteval auto
get_class_name(const char* pretty_function = __PRETTY_FUNCTION__) {
    return extract_class_name(pretty_function);
}

// TODO(ppqwqqq): Replace macro to template
#define FUNCTION_NAME() \
    get_function_name(__PRETTY_FUNCTION__)

#define CLASS_NAME() \
    get_class_name(__PRETTY_FUNCTION__)

#define FULL_FUNCTION_INFO() \
    get_caller_info(__FILE__, __LINE__, __PRETTY_FUNCTION__);

enum class TimerType {
    STANDARD,       // std
    HIGH_PRECISION, // asm
    HYBRID          // 组合使用，体现差异
};

template <TimerType Type = TimerType::HIGH_PRECISION>
struct FunctionTracer {
    FunctionInfo info;

    std::chrono::high_resolution_clock::time_point std_start_time;
    HighPrecisionTimer::TimePoint hp_start_time;

    explicit FunctionTracer(FunctionInfo func_info)
        : info{std::move(func_info)}
        {
        if constexpr (Type == TimerType::STANDARD || Type == TimerType::HYBRID) 
            std_start_time = std::chrono::high_resolution_clock::now();
        
        if constexpr (Type == TimerType::HIGH_PRECISION || Type == TimerType::HYBRID) {}
            hp_start_time = HighPrecisionTimer::now(); 
    }

    ~FunctionTracer() {
        if constexpr (Type == TimerType::STANDARD) {
            auto std_end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                std_end_time - std_start_time
            );

            std::cout << "[TRACE] " << info 
                    << " executed in " << duration.count() 
                    << " ns (std::chrono)\n";
        } else if constexpr (Type == TimerType::HIGH_PRECISION) {
            auto hp_end_time = HighPrecisionTimer::now();
            auto hp_duration = HighPrecisionTimer::Duration{hp_start_time, hp_end_time};

            // TODO(ppqwqqq): Replace this logger info async logger
            std::cout << "[TRACE] " << info << " executed in "
                      << std::fixed << std::setprecision(3)
                      << hp_duration.to_nanoseconds() << " ns ("
                      << hp_duration.get_cycles() << " cycles()\n";
        } else if constexpr (Type == TimerType::HYBRID) {
            auto std_end_time = std::chrono::high_resolution_clock::now();
            auto hp_end_time = HighPrecisionTimer::now();

            auto std_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std_end_time - std_start_time
            );

            auto hp_duration = HighPrecisionTimer::Duration{hp_start_time, hp_end_time};

            // TODO(ppqwqqq): Replace this logger info async logger
            std::cout << "[TRACE] " << info << " executed in:\n"
                      << "  std::chrono: "  << std_duration.count() << " ns\n"
                      << "  CPU cycles:  "  << std::fixed << std::setprecision(3)
                      << hp_duration.to_nanoseconds() << " ns ("
                      << hp_duration.get_cycles() << " cycles)\n"
                      << "  Difference:  "  << std::abs(static_cast<double>(
                                                    std_duration.count()) - hp_duration.to_nanoseconds()
                                               ) << " ns\n"; 
        }
    }
};

// TODO(ppqwqqq): Replace macro to template
#define TRACE_FUNCTION_STD() \
    FunctionTracer<TimerType::STANDARD> _tracer(CURRENT_FUNCTION_INFO())

#define TRACE_FUNCTION_HP() \
    FunctionTracer<TimerType::HIGH_PRECISION> _tracer(CURRENT_FUNCTION_INFO())

#define TRACE_FUNCTION_HYBRID() \
    FunctionTracer<TimerType::HYBRID> _tracer(CURRENT_FUNCTION_INFO())

// 默认
#define TRACE_FUNCTION() TRACE_FUNCTION_HP()

class PerformanceBenchmark {
private:
    std::string name;
    HighPrecisionTimer::TimePoint start_time;
    std::vector<HighPrecisionTimer::Duration> measurements;

public:
    explicit PerformanceBenchmark(std::string name)
        : name{std::move(name)}
        { start_time = HighPrecisionTimer::now(); }

    void checkpoint(const std::string& checkpoint_name = "") {
        auto end_time = HighPrecisionTimer::now();
        auto duration = HighPrecisionTimer::Duration{start_time, end_time};
        measurements.push_back(duration);

        // TODO(ppqwqqq): Replace this logger info async logger
        std::cout << "[BENCHMARK]" << name;
        if (!checkpoint_name.empty()) std::cout << " - " << checkpoint_name;
        std::cout << ": " << std::fixed << std::setprecision(3)
                  << duration.to_nanoseconds() << " ns ("
                  << duration.get_cycles() << " cycles)\n";
        start_time = end_time;
    }

    void summary() const {
        if (measurements.empty()) return;

        double total_ns = 0.0;
        std::uint64_t total_cycles = 0;

        for (const auto& measurement: measurements) {
            total_ns += measurement.to_nanoseconds();
            total_cycles += measurement.get_cycles();
        }
        // TODO(ppqwqqq): Replace this logger info async logger
        std::cout << "[BENCHMARK SUMMARY]" << name << ":\n"
                  << "  Total measurements: " << measurements.size() << "\n"
                  << "  Total time: " << std::fixed << std::setprecision(3)
                  << total_ns << " ns (" << total_cycles << " cycles)\n"
                  << "  Average time: " << (total_ns / measurements.size()) << " ns\n"
                  << "  CPU frequency: " << HighPrecisionTimer::get_cpu_frequency() << " GHz\n";
    }
};

// TODO(ppqwqqq): Replace macro to template
#define BENCHMARK(name) \
    PerformanceBenchmark _benchmark(name)

#define BENCHMARK_CHECKPOINT(name) \
    _benchmark.checkpoint(name)

#define BENCHMARK_SUMMARY() \
    _benchmark.summary()



} // namespace labelimg::core::formatter::function

#endif // FUNCTION_H