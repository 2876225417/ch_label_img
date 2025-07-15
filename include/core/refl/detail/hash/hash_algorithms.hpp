#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include "core/message_queue.hpp"
#include "core/refl/detail/hash.hpp"

namespace labelimg::core::refl::hash::algorithms {

enum class string_hash_algorithm
    : std::uint8_t {
        fnv1a,
        djb2,
        murmur3,
        crc32,
        city_hash
};

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

constexpr auto 
compute(const char* data, std::size_t length) 
noexcept -> std::size_t {
    auto hash = static_cast<std::size_t>(default_constants::offset_basis);
    for (std::size_t i = 0; i < length; ++i) {
        hash ^= static_cast<std::size_t>(static_cast<unsigned char>(data[i]));
        hash *= static_cast<std::size_t>(default_constants::prime);
    }
    return hash;
}

constexpr auto 
compute(std::string_view str) 
noexcept -> std::size_t {
    return compute(str.data(), str.length());
}

constexpr auto 
compute(const char* str) 
noexcept -> std::size_t {
    auto hash = static_cast<std::size_t>(default_constants::offset_basis);
    while (*str) {
        hash ^= static_cast<std::size_t>(static_cast<unsigned char>(*str));
        hash *= static_cast<std::size_t>(default_constants::prime);
        ++str;
    }
    return hash;
}
}  // namespace fnv1a

namespace djb2 {
/*** djb2 描述
 *   Daniel J. Bernstein 的简单哈希算法
 *   初始值: 5381
 *   公式:   hash * 33 + c
 */


static constexpr std::uint32_t initial_value = 5381;
static constexpr std::uint32_t multiplier = 33;

template <typename P>
concept IsTagPolicy = 
    std::is_empty_v<P> && 
    std::is_default_constructible_v<P>;

enum DJB2METHOD
    : std::int8_t { 
        PLUS,
        XOR
};

struct PlusPolicy { DJB2METHOD policy = DJB2METHOD::PLUS; };
struct XORPolicy  { DJB2METHOD policy = DJB2METHOD::XOR;  };

// template <IsTagPolicy ComputePolicy = PlusPolicy>
// constexpr auto compute(const char* data, std::size_t length) noexcept -> std::size_t;

// template <IsTagPolicy ComputePolicy = PlusPolicy>
// constexpr auto compute(std::string_view str) noexcept -> std::size_t;

// template <IsTagPolicy ComplutePolicy = PlusPolicy>
// constexpr auto compute(const char* str) noexcept -> std::size_t;

constexpr auto 
compute(const char* data, std::size_t length) 
noexcept -> std::size_t {
    std::size_t hash = initial_value;
    for (std::size_t i = 0; i < length; ++i) 
    { hash = // hash = hash * 33 + data[i]
        ((hash << 5) + hash) + static_cast<unsigned char>(data[i]); }
    return hash;
}

constexpr auto
compute(std::string_view str)
noexcept -> std::size_t 
{ return compute(str.data(), str.length()); }

constexpr auto
compute(const char* str)
noexcept -> std::size_t {
    std::size_t hash = initial_value;
    while (*str) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(*str);
        ++str;
    }
    return hash;
}

constexpr auto 
compute_xor(const char* data, std::size_t length) 
noexcept -> std::size_t {
    std::size_t hash = initial_value;
    for (std::size_t i = 0; i < length; ++i)
    { hash = ((hash << 5) + hash) ^ static_cast<unsigned char>(data[i]); }
    return hash;
}
} // namespace djb2 

namespace murmur3 {

} // namespace murmur3 


} // namespace labelimg::core::refl::hash::algorithms
