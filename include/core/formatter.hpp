#ifndef FORMATTER_HPP
#define FORMATTER_HPP

#include "utils/method_concepts.h"
#include <fmt/base.h>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <locale>
#include <magic_enum/magic_enum.hpp>

#include <core/async_logger.h>
#include <spanstream>
#include <sstream>
#include <thread>
#include <tuple>
#include <QDebug>

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

namespace labelimg::core::formatter {

enum class LogField: std::uint8_t {
    LEVEL         = 1 << 0,
    TIMESTAMP     = 1 << 1,
    THREAD_ID     = 1 << 2,
    FILE_INFO     = 1 << 3,
    FUNCTION_NAME = 1 << 4
};

constexpr auto operator|(LogField lhs, LogField rhs) -> LogField {
    return static_cast<LogField>(
        static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs)
    );
}

constexpr auto operator&(LogField lhs, LogField rhs) -> LogField {
    return static_cast<LogField>(
        static_cast<std::uint8_t>(lhs) & static_cast<std::uint8_t>(rhs)
    );
};

constexpr auto has_field(LogField fields, LogField field) -> bool {
    return (fields & field) == field;
}

enum class TimestampFormat: std::uint8_t {
    IOS8601,    // 2025-01-01T12:12:12.123Z
    SIMPLE,     // 2025-01-01 12:12:12
    TIME_ONLY,  // 12:12:12.123
    COMPACT,    // 20250101_121212
    RELATIVE,   // +1.234s
};



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

class TimestampFormatter {
private:
    static inline auto start_time = std::chrono::steady_clock::now();
public:
    static auto format_timestamp(TimestampFormat format) -> std::string {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ) % 1000;

        std::ostringstream oss;
        
        switch(format) {
            case TimestampFormat::IOS8601: {
                oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S");
                oss << "." << std::setfill('0') << std::setw(3) << ms.count() << 'Z';
                break;
            }
            case TimestampFormat::SIMPLE: {
                oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
                break;
            }
            case TimestampFormat::TIME_ONLY: {
                oss << std::put_time(std::localtime(&time_t), "%H%M%S");
                oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
                break;
            }
            case TimestampFormat::COMPACT: {
                oss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
                break;
            }
            case TimestampFormat::RELATIVE: {
                auto steady_now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    steady_now - start_time
                );
                oss << '+' << elapsed.count() / 1000.0 << 's';
                break;
            }
        }
        return oss.str();
    }

    static auto format_timestamp_fixed_width(TimestampFormat format, size_t width = 0) -> std::string {
        auto timestamp = format_timestamp(format);
        if (width > 0 && timestamp.length() < width) timestamp.resize(width, ' ');
        return timestamp;
    }
};

inline auto format_thread_id() -> std::string {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    return oss.str();
}

inline auto format_file_info(const char* file, int line) -> std::string {
    if (!file) return "";
    
    // 只保留文件名
    const char* filename = strchr(file, '/');
    if (!filename) filename = strchr(file, '\\');
    if (!filename) filename = file;
    else filename++;
    
    return std::string(filename) + ":" + std::to_string(line);
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

struct LogFormatConfig {
    LogField enabled_fields = LogField::LEVEL | LogField::TIMESTAMP;
    TimestampFormat timestamp_format = TimestampFormat::SIMPLE;
    std::string field_separator = " | ";
    std::string level_bracket_left = "[";
    std::string level_bracket_right = "]";
    size_t max_line_width = 80;
    bool enable_colors = true;

    size_t level_width = 11;
    size_t timestamp_width = 19;
    size_t thread_id_width = 8;
    size_t file_info_width = 20;
};

struct log_attributes {
    console_style::PresetStyle style_;
    std::string_view           tag_;
    std::string                timestamp_;
    std::string                thread_id_;
    std::string                file_info_;
    std::string                function_name;
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
    static constexpr auto create_attributes(
        const LogFormatConfig& config = {},
        const char* file = nullptr,
        int line = 0,
        const char* function = nullptr
    ) -> log_attributes {
        log_attributes attrs;
        attrs.style_ = level2style(level);
        attrs.tag_ = generate_tag_object;
        
        if (has_field(config.enabled_fields, LogField::TIMESTAMP)) {
            attrs.timestamp_ = detail::TimestampFormatter::format_timestamp_fixed_width(
                config.timestamp_format, config.timestamp_width
            );
        }

        if (has_field(config.enabled_fields, LogField::THREAD_ID)) {
            attrs.thread_id_ = detail::format_thread_id();
            if (attrs.thread_id_.length() > config.thread_id_width) {
                attrs.thread_id_ = attrs.thread_id_.substr(0, config.thread_id_width);
            }
        }

        if (has_field(config.enabled_fields, LogField::FILE_INFO) && file) {
            attrs.file_info_ = detail::format_file_info(file, line);
            if (attrs.file_info_.length() > config.file_info_width) {
                attrs.file_info_ = "..." + attrs.file_info_.substr(
                    attrs.file_info_.length() - config.file_info_width + 3
                );
            }
        }
        
        if (has_field(config.enabled_fields, LogField::FUNCTION_NAME) && function) {
            attrs.function_name = function;
        }

        return attrs;
    }

    static constexpr log_attributes value {
        .style_ = level2style(level),
        .tag_   = generate_tag_object
    };
};


class Formatter {
private:
    LogFormatConfig m_config;
public:
    explicit Formatter(const LogFormatConfig& config = {}) : m_config{std::move(config)} {}
    
    void set_config(const LogFormatConfig& config) { m_config = config; }
    auto get_config() -> const LogFormatConfig& { return m_config; }

    template <LogLevel level>
    auto format_message(
        std::string_view message,
        const char* file = nullptr,
        int line = 0,
        const char* function = nullptr
    ) const -> std::string {
        auto attrs = log_level_traits<level>::create_attributes(m_config, file, line, function);
        
        std::ostringstream prefix;

        if (m_config.enable_colors) {
            prefix << console_style::get_preset_style_code<attrs.style_>().c_str();
        }

        if (has_field(m_config.enabled_fields, LogField::TIMESTAMP) && !attrs.timestamp_.empty()) {
            prefix << attrs.timestamp_ << m_config.field_separator;
        }

        if (has_field(m_config.enabled_fields, LogField::THREAD_ID) && !attrs.thread_id_.empty()) {
            prefix << "[T:" << attrs.thread_id_ << "]" << m_config.field_separator;
        }

        if (has_field(m_config.enabled_fields, LogField::LEVEL)) {
            prefix << attrs.tag_ << m_config.field_separator;
        }

        if (has_field(m_config.enabled_fields, LogField::FILE_INFO) && !attrs.file_info_.empty()) {
            prefix << "[" << attrs.file_info_ << "]" << m_config.field_separator;
        }

        if (has_field(m_config.enabled_fields, LogField::FUNCTION_NAME) && !attrs.function_name_.empty()) {
            prefix << attrs.function_name_ << "()" << m_config.field_separator;
        }
        
        if (m_config.enable_colors) {
            prefix << console_style::get_preset_style_code<console_style::PresetStyle::C_RESET>().c_str();
        }

        std::string prefix_str = prefix.str();

        auto lines = detail::split_string_by_width(message, m_config.max_line_width);
        std::ostringstream result;

        if (!lines.empty()) {
            result << prefix_str << lines[0] << '\n';
        }

        if (lines.size() > 1) {
            std::string indent = detail::create_indent_from(prefix_str);
            for (size_t i = 1; i < lines.size(); ++i) {
                if (m_config.enable_colors) 
                    result << console_style::get_preset_style_code<attrs.style_>().c_str();
                result << indent;
                if (m_config.enable_colors) {
                    result << console_style::get_preset_style_code<console_style::PresetStyle::C_RESET>().c_str();
                }
                result << lines[i] << '\n';
            }
        }
        return result.str();
    } 
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
}

#endif // FORMATTER_HPP
