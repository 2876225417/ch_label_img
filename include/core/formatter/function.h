#ifndef FUNCTION_H
#define FUNCTION_H

#include "core/formatter/reflection.h"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <array>


#if __cplusplus >= 202002L
#include <source_location>
#define HAS_SOURCE_LOCATION 1
#else
#define HAS_SOURCE_LOCATION 0
#endif

namespace labelimg::core::formatter::function {
using reflection::find_char;
using reflection::rfind_char;

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

#define FUNCTION_NAME() \
    get_function_name(__PRETTY_FUNCTION__)

#define CLASS_NAME() \
    get_class_name(__PRETTY_FUNCTION__)

#define FULL_FUNCTION_INFO() \
    get_caller_info(__FILE__, __LINE__, __PRETTY_FUNCTION__);

struct FunctionTracer {
    FunctionInfo info;
    std::chrono::high_resolution_clock::time_point start_time;
    
    explicit FunctionTracer(FunctionInfo func_info)
        : info{std::move(func_info)}
        , start_time{std::chrono::high_resolution_clock::now()}
        { }

    ~FunctionTracer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    }
};

#define TRACE_FUNCTION() \
    FunctionTracer _tracer(CURRENT_FUNCTION_INFO())


} // namespace labelimg::core::formatter::function

#endif // FUNCTION_H