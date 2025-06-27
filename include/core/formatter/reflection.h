#ifndef REFLECTION_H
#define REFLECTION_H


namespace labelimg::core::formatter::reflection {

template <size_t N>
struct compile_time_string {
    std::array<char, N + 1> data{};

    consteval compile_time_string() = default;

    consteval explicit compile_time_string(const char (&str)[N + 1]) {
        std::copy_n(str, N + 1, data.begin());
    }

    consteval explicit compile_time_string(std::string_view sv) {
        std::copy(sv.data(), N, data.begin());
        data[N] = '\0';
    }

    [[nodiscard]] constexpr auto 
    size() 
    const noexcept -> size_t { return N; }
    [[nodiscard]] constexpr auto 
    c_str() 
    const noexcept -> const char* { return data.data(); }
    [[nodiscard]] constexpr explicit 
    operator std::string_view() 
    const noexcept { return {data.data(), N}; }
};

template <size_t N>
compile_time_string(const char (&)[N]) -> compile_time_string<N - 1>; 

consteval auto find_char(std::string_view str, char ch, size_t start = 0) -> size_t {
    for (size_t i = start; i < str.size(); ++i) if (str[i] == ch) return i;
    return std::string_view::npos;
}

consteval auto rfind_char(std::string_view str, char ch, size_t start = std::string_view::npos) -> size_t {
    if (start == std::string_view::npos) start = str.size();
    for (size_t i = start; i > 0; --i) if (str[i - 1] == ch) return i - 1;
    return std::string_view::npos;
}

} // namespace labelimg::core::formatter::reflection

#endif // REFLECTION_H