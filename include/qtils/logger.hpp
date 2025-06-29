#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "qtils_pch.h"
#include "utils/method_concepts.h"
#include <fmt/base.h>
#include <magic_enum/magic_enum.hpp>

#include <QDebug>
#include <core/async_logger.h>
#include <tuple>

#if defined (USE_STD_FMT)
#include <format>
namespace app_format_ns = std;
#elif defined (USE_EXTERNAL_FMT)
#include <fmt/core.h>
#include <fmt/format.h>
namespace app_format_ns = fmt;
#else
#error "Required std::fmt or fmt for format output."
#endif

template <typename... Args>
using app_format_string = app_format_ns::format_string<Args...>;

namespace labelimg::qtils::logger {

namespace detail {
template <size_t N>
struct fixed_string {
    std::array<char, N + 1> data_{};
    
    constexpr fixed_string() = default;
#if __cplusplus <= 202002L
    consteval fixed_string(const char (&str)[N + 1]) {
        for (size_t i = 0; i <= N; ++i) 
            data_[i] = str[i];
    }

    constexpr fixed_string(const std::array<char, N + 1>& str) {
        for (size_t i = 0; i <= N; ++i) 
            data_[i] = str[i];
    }

    constexpr fixed_string(std::string_view sv) {
        for (size_t i = 0; i < N; ++i) 
            data_[i] = sv[i];
    }    

    template <size_t M>
    constexpr auto operator+ (const fixed_string<M>& other) const {
        fixed_string<N + M> result;
        // Copy fixed_string<N>
        for (size_t i = 0; i < N; ++i) 
            result.data_[i] = this->data_[i];
        // Copy fixed_string<M>
        for (size_t i = 0; i < M; ++i) 
            result.data_[i + N] = other.data_[i];
        return result;
    } 
#else
    consteval fixed_string(const char (&str)[N + 1]) {
        std::copy_n(str, N + 1, data_.begin());
    }

    constexpr fixed_string(const std::array<char, N + 1>& str) {
        std::copy_n(str.data(), N + 1, data_.begin());    
    }

    constexpr fixed_string(const std::string_view& sv) {
        std::copy_n(sv.data(), N, data_.begin());
    }

    template <size_t M>
    constexpr auto operator+ (const fixed_string<M>& other) const {
        fixed_string<N + M> result;
        // Copy fixed_string<N>
        std::copy_n(this->data_.data(), N, result.data_.data());
        // Copy fixed_string<M>
        std::copy_n(other.data_.data(), M, result.data_.data() + N);
        return result;
    }
#endif // __cplusplus <= 202002L

    [[nodiscard]] constexpr auto size()  const -> size_t { return N; }
    [[nodiscard]] constexpr auto c_str() const -> const char* { return data_.data(); }
    constexpr operator std::string_view() const { return { data_.data(), N}; }
};

template <size_t N> // CTAD
fixed_string(const char (&str)[N]) -> fixed_string<N - 1>;

template <std::uintmax_t V>
consteval auto to_string() {
    if constexpr (V == 0) return fixed_string("0");
    else {
        constexpr auto num_digits = [] {
            size_t count = 0;
            for (auto i = V; i > 0 ; i /= 10) count++;
            return count;
        }();
        fixed_string<num_digits> result{};
        auto val = V;
        for (size_t i = 0; i < num_digits; ++i) {
            result.data_[num_digits - 1 - i] = '0' + (val % 10);
            val /= 10;
        }
        return result;
    }
}

consteval auto build_style_string() { return fixed_string(""); }

template <auto Head, auto... Tail>
consteval auto build_style_string() {
    constexpr auto val = magic_enum::enum_integer(Head);
    
    auto head_str = to_string<val>();
    if constexpr (sizeof...(Tail) > 0) 
        return head_str + fixed_string(";") + build_style_string<Tail...>();
    else
        return head_str;
}

// 计算 ASCII 和 Unicode 字符的个数
[[nodiscard]] inline auto 
get_visual_width(std::string_view str) -> size_t {
    size_t byte_len = str.length();
    size_t num_unicode_chars = 0;
    for (size_t i = 0; i < byte_len; ) {
        if ((str[i] & 0x80) == 0) // ASCII 
            i += 1; 
        else {  // 多字节字符
            num_unicode_chars++;
            if      ((str[i] & 0xE0) == 0xC0) i += 2;
            else if ((str[i] & 0xF0) == 0xE0) i += 3;
            else if ((str[i] & 0xF8) == 0XF0) i += 4;
            else/* 对于非 UTF-8 字符的回滚处理 */ i += 1;
        }
    }
    return (byte_len - num_unicode_chars * 2);
}

[[nodiscard]] inline auto 
split_string_by_width(std::string_view str, size_t max_width) 
-> decltype(auto) {
    std::vector<std::string> lines;

    // 提前检测
    if ((str.empty())) {
        lines.emplace_back("");
        return lines;
    }

    size_t current_pos = 0;
    while (current_pos < str.length()) {
        lines.emplace_back(str.substr(current_pos, max_width));
        current_pos += max_width;
    }

    return lines;
}

[[nodiscard]] inline auto
create_indent_from(std::string_view reference_str) 
-> decltype(auto) {
    size_t width = get_visual_width(reference_str);
    return std::string(width, ' ');
};


} // namespace detail


namespace console_style {
#if defined(USE_CPP_COLORED_DEBUG_OUTPUT)

enum class Attribute: std::int8_t {
    RESET     = 0,  // 重置样式
    BOLD      = 1,  // 粗体或增亮
    DIM       = 2,  // 暗淡
    UNDERLINE = 4,  // 下划线
    BLINK     = 5,  // 闪烁
    REVERSE   = 7,  // 反色
    HIDDEN    = 8   // 隐藏
};

enum class ForegroundColor: std::int8_t {
    DEFAULT   = 39,
    BLACK     = 30, RED   = 31, GREEN   = 32,
    YELLOW    = 33, BLUE  = 34, MAGENTA = 35,
    CYAN      = 36, WHITE = 37
};

enum class BackgroundColor: std::int8_t {
    DEFAULT   = 49,
    BLACK     = 40, RED   = 41, GREEN   = 42,
    YELLOW    = 43, BLUE  = 44, MAGENTA = 45,
    CYAN      = 46, WHITE = 47
};

enum class PresetStyle: std::int8_t {
    C_RESET,
    C_BLACK,  C_RED,  C_GREEN,
    C_YELLOW, C_BLUE, C_MAGENTA,
    C_CYAN,   C_WHITE,
    B_RESET,
    B_BLACK,  B_RED,  B_GREEN,
    B_YELLOW, B_BLUE, B_MAGENTA,
    B_CYAN,   B_WHITE,
    U_RESET,
    U_BLACK,  U_RED,  U_GREEN,
    U_YELLOW, U_BLUE, U_MAGENTA,
    U_CYAN,   U_WHITE,
};

template <auto... Styles>
consteval auto apply() {
    if constexpr (sizeof...(Styles) == 0) {
        return detail::fixed_string("\x1b[0m");
    } else {
        auto style_codes = detail::build_style_string<Styles...>();
        return detail::fixed_string("\x1b[") + style_codes + detail::fixed_string("m");
    }
}

template <PresetStyle Preset>
consteval auto get_preset_style_code() noexcept {
    if constexpr (Preset == PresetStyle::C_RESET ||
                  Preset == PresetStyle::B_RESET ||
                  Preset == PresetStyle::U_RESET
                ) { return apply<Attribute::RESET>(); }
    
    else if constexpr (Preset == PresetStyle::C_BLACK)   { return apply<ForegroundColor::BLACK>(); }
    else if constexpr (Preset == PresetStyle::C_RED)     { return apply<ForegroundColor::RED>(); }
    else if constexpr (Preset == PresetStyle::C_GREEN)   { return apply<ForegroundColor::GREEN>(); }
    else if constexpr (Preset == PresetStyle::C_YELLOW)  { return apply<ForegroundColor::YELLOW>(); }
    else if constexpr (Preset == PresetStyle::C_BLUE)    { return apply<ForegroundColor::BLUE>(); }
    else if constexpr (Preset == PresetStyle::C_MAGENTA) { return apply<ForegroundColor::MAGENTA>(); }
    else if constexpr (Preset == PresetStyle::C_CYAN)    { return apply<ForegroundColor::CYAN>(); }
    else if constexpr (Preset == PresetStyle::C_WHITE)   { return apply<ForegroundColor::WHITE>(); }
    // 粗体颜色
    else if constexpr (Preset == PresetStyle::B_BLACK)   { return apply<Attribute::BOLD, ForegroundColor::BLACK>(); }
    else if constexpr (Preset == PresetStyle::B_RED)     { return apply<Attribute::BOLD, ForegroundColor::RED>(); }
    else if constexpr (Preset == PresetStyle::B_GREEN)   { return apply<Attribute::BOLD, ForegroundColor::GREEN>(); }
    else if constexpr (Preset == PresetStyle::B_YELLOW)  { return apply<Attribute::BOLD, ForegroundColor::YELLOW>(); }
    else if constexpr (Preset == PresetStyle::B_BLUE)    { return apply<Attribute::BOLD, ForegroundColor::BLUE>(); }
    else if constexpr (Preset == PresetStyle::B_MAGENTA) { return apply<Attribute::BOLD, ForegroundColor::MAGENTA>(); }
    else if constexpr (Preset == PresetStyle::B_CYAN)    { return apply<Attribute::BOLD, ForegroundColor::CYAN>(); }
    else if constexpr (Preset == PresetStyle::B_WHITE)   { return apply<Attribute::BOLD, ForegroundColor::WHITE>(); }
    // 下划线颜色
    else if constexpr (Preset == PresetStyle::U_BLACK)   { return apply<Attribute::UNDERLINE, ForegroundColor::BLACK>(); }
    else if constexpr (Preset == PresetStyle::U_RED)     { return apply<Attribute::UNDERLINE, ForegroundColor::RED>(); }
    else if constexpr (Preset == PresetStyle::U_GREEN)   { return apply<Attribute::UNDERLINE, ForegroundColor::GREEN>(); }
    else if constexpr (Preset == PresetStyle::U_YELLOW)  { return apply<Attribute::UNDERLINE, ForegroundColor::YELLOW>(); }
    else if constexpr (Preset == PresetStyle::U_BLUE)    { return apply<Attribute::UNDERLINE, ForegroundColor::BLUE>(); }
    else if constexpr (Preset == PresetStyle::U_MAGENTA) { return apply<Attribute::UNDERLINE, ForegroundColor::MAGENTA>(); }
    else if constexpr (Preset == PresetStyle::U_CYAN)    { return apply<Attribute::UNDERLINE, ForegroundColor::CYAN>(); }
    else if constexpr (Preset == PresetStyle::U_WHITE)   { return apply<Attribute::UNDERLINE, ForegroundColor::WHITE>(); }
    else {
        static_assert(std::is_void_v<decltype(Preset)>, "Unsupported preset style provided!");
        return detail::fixed_string("");
    }
}

#else
template <PresetStyle Preset>
consteval auto get_preset_style_code()
noexcept { return detail::fixed_string<1>(""); }
#endif // defined(USE_CPP_COLORED_DEBUG_OUTPUT)

} // namespace console_style

#ifdef BUILD_TESTS
constexpr bool is_test_build = true;
#else
constexpr bool is_test_build = false;
#endif

using LoggerRetType = std::conditional_t<is_test_build, bool, void>;

#if defined(USE_STD_FMT) || defined(USE_EXTERNAL_FMT)

enum class LogLevel: std::int8_t {
    INFO,
    SUCCESS,
    WARNING,
    ERROR,
    FATAL_ERROR
};

constexpr auto 
level2style(LogLevel level) 
noexcept -> console_style::PresetStyle {
    switch (level) {
        case LogLevel::INFO:        return console_style::PresetStyle::B_GREEN;
        case LogLevel::SUCCESS:     return console_style::PresetStyle::B_CYAN;
        case LogLevel::WARNING:     return console_style::PresetStyle::B_YELLOW;
        case LogLevel::ERROR:       return console_style::PresetStyle::B_RED;
        case LogLevel::FATAL_ERROR: return console_style::PresetStyle::U_RED;
        default:                    return console_style::PresetStyle::C_RESET;
    }
}

struct log_attributes {
    console_style::PresetStyle style_;
    std::string_view           tag_;
};

template <LogLevel level>
struct log_level_traits {
private:
    static constexpr auto 
    generate_tag() { /* 如 [SUCCESS]       | Message    */
                     /*   [INFO]          | Message    */
        constexpr auto name_sv = magic_enum::enum_name<level>();
        constexpr size_t name_sv_len = name_sv.size();

        std::array<char, name_sv_len + 1> processed_name_buffer = {};
        for (size_t i = 0; i < name_sv_len; ++i) 
            processed_name_buffer[i] = (name_sv[i] == '_') ? ' ' : name_sv[i];

        detail::fixed_string<name_sv_len> processed_name(processed_name_buffer.data());
        constexpr size_t max_name_len = 11;
        constexpr size_t padding_size = max_name_len > name_sv_len ? max_name_len - name_sv_len : 0;

        detail::fixed_string<1> left_bracket = std::string_view("[");
        detail::fixed_string<1> right_bracket = std::string_view("]");

        constexpr std::string_view spaces("            ");

        // 从 12 个空格中取出对应数量的空格
        detail::fixed_string<padding_size> padding(std::string_view(spaces.data(), padding_size));
        detail::fixed_string<1> divider(std::string_view("|"));
        constexpr size_t divider_padding_size = 3;
        detail::fixed_string<divider_padding_size> divider_padding(std::string_view(spaces.data(), divider_padding_size));


        return left_bracket + processed_name + right_bracket + padding + divider + divider_padding;
    }
    static constexpr auto generate_tag_object = generate_tag();
public:
    static constexpr log_attributes value {
        .style_ = level2style(level),
        .tag_   = generate_tag_object
    };
};


template <typename T>
struct fmt_arg_type { using type = T; };

template <>
struct fmt_arg_type<QString> { using type = std::string; };

template <typename T>
using fmt_arg_type_t = typename fmt_arg_type<std::decay_t<T>>::type;

template <typename T>
[[nodiscard]] auto transform_arg_for_fmt(T&& arg) {
    using DecayedT = std::decay_t<T>;
    
    if constexpr (std::is_same_v<DecayedT, QString>) {
        return arg.toStdString();
    } else {
        return std::forward<T>(arg);
    }
}

template<LogLevel level, typename... Args>
auto logg( app_format_string<fmt_arg_type_t<Args>...> fmt_str
         , Args&&... args
         ) -> decltype(auto) {
    constexpr auto style = log_level_traits<level>::value.style_;
    constexpr auto tag   = log_level_traits<level>::value.tag_;
    
    std::string formatted_message = app_format_ns::format(
        fmt_str, 
        transform_arg_for_fmt(std::forward<Args>(args))...
    );
    
#if defined(TERM_OUTPUT_MESSAGE_MAX_LENGTH)
    constexpr size_t MAX_LINE_WIDTH = TERM_OUTPUT_MESSAGE_MAX_LENGTH;
#else
    constexpr size_t MAX_LINE_WIDTH = 80;
#endif

    auto lines = detail::split_string_by_width(formatted_message, MAX_LINE_WIDTH);

    using namespace labelimg::core::logger;

    if (!lines.empty())
        async_log << console_style::get_preset_style_code<style>().c_str() << tag
                  << console_style::get_preset_style_code<console_style::PresetStyle::C_RESET>().c_str()
                  << lines[0];
    
    if (lines.size() > 1) {
        std::string indent = detail::create_indent_from(tag);
        for (size_t i = 1; i < lines.size(); ++i)     
                async_log << console_style::get_preset_style_code<style>().c_str() << indent
                  << console_style::get_preset_style_code<console_style::PresetStyle::C_RESET>().c_str()
                  << lines[i];
    }


    if constexpr (is_test_build) 
        return static_cast<LoggerRetType>(LoggerRetType(true));
}



#endif
} // namespace labelimg::qtils::logger

#endif // LOGGER_HPP
