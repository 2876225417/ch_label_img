#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <magic_enum/magic_enum.hpp>

#include <QDebug>


#if defined (USE_STD_FMT)
#include <format>
namespace app_format_ns = std;
template <typename... Args>
using app_format_string = std::format_string<Args...>;
template <typename... Args>
auto make_format_args(Args&&... args) {
    return std::make_format_args(std::forward<Args>(args)...);
}
#elif defined (USE_EXTERNAL_FMT)
#include <fmt/core.h>
#include <fmt/format.h>
namespace app_format_ns = fmt;
template <typename... Args>
using app_format_string = fmt::format_string<Args...>;
template <typename... Args>
auto make_format_args(Args&&... args) {
    return fmt::make_format_args(std::forward<Args>(args)...);
}
#else
#error "Required std::fmt or fmt for format output."
#endif

template <typename... Args>
using app_format_string = app_format_ns::format_string<Args...>;
using app_format_ns::make_format_args;

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
        for (size_t i = 0; i < M: ++i) 
            result.data_[i + N] = other->data_[i];
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
    generate_tag() { /* 如 [SUCCESS] */
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
        // 从 12 个空格中取出对应数量的空格
        detail::fixed_string<padding_size> padding(std::string_view("            ", padding_size));
        
        return left_bracket + processed_name + right_bracket + padding;
    }
    static constexpr auto generate_tag_object = generate_tag();
public:
    static constexpr log_attributes value {
        .style_ = level2style(level),
        .tag_   = generate_tag_object
    };
};


template<LogLevel level, typename... Args>
auto log( app_format_string<Args...> fmt_str
        , Args&&... args) -> LoggerRetType
        {
    constexpr auto style = log_level_traits<level>::value.style_;
    constexpr auto tag   = log_level_traits<level>::value.tag_;
    
    std::cout << console_style::get_preset_style_code<style>().c_str() << tag
              << console_style::get_preset_style_code<console_style::PresetStyle::C_RESET>().c_str()
              << app_format_ns::vformat(fmt_str.get(), make_format_args(std::forward<Args>(args)...))
              << std::endl;

    if constexpr (is_test_build) return true;
}

#endif
} // namespace labelimg::qtils::logger

#endif // LOGGER_HPP