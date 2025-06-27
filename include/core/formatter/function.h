#ifndef FUNCTION_H
#define FUNCTION_H

#include "core/formatter/reflection.h"
#include <string_view>
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

consteval auto extract_function_name(std::string_view pretty_function) -> std::string_view {
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




} // namespace labelimg::core::formatter::function

#endif // FUNCTION_H