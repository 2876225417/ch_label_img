#pragma once

#include <cstddef>
namespace labelimg::core::refl::detail {

namespace fnv1a {
/*** FNV-1a 描述
 *   offset_basis 初始的哈希值
 *   prime: FNV   用于散列的质数
 */

template <size_t N>
struct constants;

template <>
struct constants<4> {
    static constexpr std::uint32_t offset_basis = 2166136261u;
    static constexpr std::uint32_t prime = 16777619u;
    using hash_type = std::uint32_t;
};

template <>
struct constants<8> {
    static constexpr std::uint64_t offset_basis = 14695981039346656037ull;
    static constexpr std::uint64_t prime = 1099511628211ull;
    using hash_type = std::uint64_t;
};

using default_constants = constants<sizeof(size_t)>;
using hash_type = typename default_constants::hash_type;
}  // namespace fnv1a 

constexpr auto 
string_hash(const char* str, std::size_t length) 
noexcept -> std::size_t {
    auto hash = 
        static_cast<std::size_t>(fnv1a::default_constants::offset_basis);
    for (std::size_t i = 0; i < length; ++i) {
        hash ^= static_cast<std::size_t>(static_cast<unsigned char>(str[i]));
        hash *= static_cast<std::size_t>(fnv1a::default_constants::prime);
    }
    return hash;
}

constexpr auto
string_hash(const char* str)
noexcept -> std::size_t {
    auto hash = 
        static_cast<std::size_t>(fnv1a::default_constants::offset_basis);

    while (*str) {
        hash ^= static_cast<std::size_t>(static_cast<unsigned char>(*str));
        hash *= static_cast<std::size_t>(fnv1a::default_constants::prime);
        ++str;
    }

    return hash;
};

constexpr auto
string_hash(std::string_view str) 
noexcept -> std::size_t {
    return string_hash(str.data(), str.length());
}

template <typename T> constexpr auto 
type_name_hash() 
noexcept -> std::size_t { // 根据类名计算哈希值
#if defined (__GNUC__) || defined (__clang__)
    return string_hash(__PRETTY_FUNCTION__);
#elif defined (_MSC_VER_)
    return string_hash(__FUNCSIG__);
#else // 回退到 typeid, 但是运行时,编译期不使用
    return std::hash<const char*>{}(typeid(T).name);
#endif
}

template <typename T> inline auto 
type_address_hash() 
noexcept -> std::size_t { // 根据运行类型地址计算哈希(运行时)
    static char type_id = 0;
    return reinterpret_cast<std::size_t>(&type_id);
}

template <typename T> constexpr auto
type_hash() 
noexcept -> std::size_t {
    if constexpr (requires {type_name_hash<T>(); }) {
        return type_name_hash<T>();
    } else {
        return 0;
    }
}

template <typename T> inline auto 
runtime_type_hash() 
noexcept -> std::size_t {
    return type_address_hash<T>();
}


}  // namespace labelimg::core::refl::detail