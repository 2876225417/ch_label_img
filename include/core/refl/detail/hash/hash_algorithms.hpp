#pragma once

#include <sys/types.h>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include "core/message_queue.hpp"
#include "core/refl/detail/hash.hpp"

namespace labelimg::core::refl::hash::algorithms {

enum class StringHashAlgo
    : std::uint8_t {
        fnv1a,
        djb2,
        murmur3,
        crc32,
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
/*** MurmurHash3 描述
 *   Austin Appleby 的高性能非加密哈希算法
 */

// MurmurHash3 常数
// 32 位
static constexpr std::uint32_t c1_32 = 0xcc9e2d51;
static constexpr std::uint32_t c2_32 = 0x1b873593;
static constexpr std::uint32_t r1_32 = 15;
static constexpr std::uint32_t r2_32 = 13;
static constexpr std::uint32_t m_32  = 5;
static constexpr std::uint32_t n_32  = 0xe6546b64;

// 64 位
static constexpr std::uint64_t c1_64 = 0x87c37b91114253d5ULL;
static constexpr std::uint64_t c2_64 = 0x4cf5ad432745937fULL;
static constexpr std::uint64_t r1_64 = 31;
static constexpr std::uint64_t r2_64 = 27;
static constexpr std::uint64_t m_64  = 5;
static constexpr std::uint64_t n_64  = 0x52dce729; 

constexpr auto
rotl32(std::uint32_t x, std::int8_t r)
noexcept -> std::uint32_t 
{ return (x << r) | (x >> (32 - r)); }

constexpr auto
rotl64(std::uint64_t x, std::int16_t r)
noexcept -> std::uint64_t 
{ return (x << r) | (x >> (64 - r)); }

constexpr auto 
fmix32(std::uint32_t h)
noexcept -> std::uint32_t {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h; 
}

constexpr auto
fmix64(std::uint64_t k) 
noexcept -> std::uint64_t {
    k ^= k >> 33;
    k *= 0xff51afd7ed558ccdULL;
    k ^= k >> 33;
    k *= 0xc4ceb9f31a85ec53ULL;
    k ^= k >> 33;
    return k;
}

constexpr auto
compute32(const char* data, std::size_t length, std::uint32_t seed = 0)
noexcept -> std::uint32_t {
    const std::size_t nblocks = length / 4;
    std::uint32_t h1 = seed;
    
    // 处理 4 字节块
    for (std::size_t i = 0; i < nblocks; ++i) {
        std::uint32_t k1 = 0;

        k1 |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[i * 4 + 0])) << 0;
        k1 |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[i * 4 + 1])) << 8;
        k1 |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[i * 4 + 2])) << 16;
        k1 |= static_cast<std::uint32_t>(static_cast<unsigned char>(data[i * 4 + 3])) << 24;
        
        k1 *= c1_32;
        k1 = rotl32(k1, r1_32);
        k1 *= c2_32;

        h1 ^= k1;
        h1 = rotl32(h1, r2_32);
        h1 = h1 * m_32 + n_32;
    }

    // 处理剩余字符
    const std::size_t tail_start = nblocks * 4;
    std::uint32_t k1 = 0;

    switch (length & 3) {
        case 3: 
            k1 ^= static_cast<std::uint32_t>(static_cast<unsigned char>(data[tail_start + 2])) << 16; 
            [[fallthrough]];
        case 2:
            k1 ^= static_cast<std::uint32_t>(static_cast<unsigned char>(data[tail_start + 1])) << 8;
            [[fallthrough]];
        case 1:
            k1 ^= static_cast<std::uint32_t>(static_cast<unsigned char>(data[tail_start + 0]));
            k1 *= c1_32;
            k1 = rotl32(k1, r1_32);
            k1 *= c2_32;
            h1 ^= k1;
    }

    h1 ^= static_cast<std::uint32_t>(length);
    h1 = fmix32(h1);

    return h1;
} 

constexpr auto // 简化ed实现
compute64(const char* data, std::size_t length, std::uint64_t seed = 0) 
noexcept -> std::uint64_t {
    const std::size_t nblocks = length / 8;
    std::uint64_t h1 = seed;

    // 处理 8 字节块
    for (std::size_t i = 0; i < nblocks; ++i) {
        std::uint64_t k1 = 0;
        for (int j = 0; j < 8; ++j)
        { k1 |= static_cast<std::uint64_t>(static_cast<unsigned char>(data[i * 8 + j])) << (j * 8); }
        
        k1 *= c1_64;
        k1 = rotl64(k1, r1_64);
        k1 *= c2_64;

        h1 ^= k1;
        h1 = rotl64(h1, r2_64);
        h1 = h1 * m_64 + n_64;
    }

    // 处理剩余字节
    const std::size_t tail_start = nblocks * 8;
    std::uint64_t k1 = 0;

    const std::size_t remaining = length & 7;
    for (std::size_t i = 0; i < remaining; ++i) 
    { k1 |= static_cast<std::uint64_t>(static_cast<unsigned char>(data[tail_start + i])) << (i * 8); }

    if (remaining > 0) {
        k1 *= c1_64;
        k1 = rotl64(k1, r1_64);
        k1 *= c2_64;
        h1 ^= k1;
    }

    h1 ^= static_cast<std::uint64_t>(length);
    h1 = fmix64(h1);

    return h1;
}

constexpr auto 
compute(const char* data, std::size_t length, std::uint32_t seed = 0)
noexcept -> std::size_t {
    if constexpr (sizeof(std::size_t) == 8) 
    { return static_cast<std::size_t>(compute64(data, length, seed)); }
    else 
    { return static_cast<std::size_t>(compute32(data, length, seed)); }
}

constexpr auto 
compute(std::string_view str, std::uint32_t seed = 0)
noexcept -> std::size_t 
{ return compute(str.data(), str.length()); }

} // namespace murmur3 

namespace crc32 {
/*** crc32 描述
 *   循环冗余校验算法
 */

// CRC32 多项式(IEEE 802.3)
static constexpr std::uint32_t polynomial = 0xedb88320;

constexpr auto
generate_crc32_table()
noexcept -> std::array<std::uint32_t, 256> {
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t crc = i;
        for (int j = 0; j < 8; ++j) {
            if (crc & 1) crc = (crc >> 1) ^ polynomial;
            else crc >>= 1;
        }
        table[i] = crc;
    }
    return table;
}

static constexpr auto crc32_table = generate_crc32_table();

constexpr auto 
compute(const char* data, std::size_t length)
noexcept -> std::size_t {
    std::uint32_t crc = 0xffffffff;
    for (std::size_t i = 0; i < length; ++i)
    { crc = crc32_table[(crc ^ static_cast<unsigned char>(data[i])) & 0xff] ^ (crc >> 8); }
    return static_cast<std::size_t>(crc ^ 0xffffffff);
}

constexpr auto 
compute(std::string_view str) 
noexcept -> std::size_t 
{ return compute(str.data(), str.length()); }

} // namespace crc32 


template <typename Algorithm>
concept HashAlgorithm = requires(const char* data, std::size_t len, std::string_view sv) {
    { Algorithm::compute(data, len) } -> std::convertible_to<std::size_t>;
    { Algorithm::compute(sv) } -> std::convertible_to<std::size_t>;
} && requires {
    { Algorithm::name } -> std::convertible_to<const char*>;
    { Algorithm::algorithm_id } -> std::convertible_to<StringHashAlgo>;
};

template <StringHashAlgo AlgorithmID>
struct AlgorithmBase {
    static constexpr StringHashAlgo algorithm_id = algorithm_id;
    static constexpr bool is_compile_time = true;
    static constexpr bool is_cryptographic = false;
};


struct fnv1a_algorithm: AlgorithmBase<StringHashAlgo::fnv1a> {
    static constexpr const char* name = "FNV-1a";
    static constexpr int quality_score = 7;

    static constexpr auto 
    compute(const char* data, std::size_t length) 
    noexcept -> std::size_t 
    { return fnv1a::compute(data, length); }

    static constexpr auto 
    compute(std::string_view str) 
    noexcept -> std::size_t 
    { return fnv1a::compute(str);}

    static constexpr auto 
    compute_literal(const char* str) 
    noexcept -> std::size_t 
    { return fnv1a::compute(str); }
};

struct djb2_algorithm: AlgorithmBase<StringHashAlgo::djb2> {
    static constexpr const char* name = "DJB2";
    static constexpr int quality_score = 6;

    static constexpr auto
    compute(const char* data, std::size_t length)
    noexcept -> std::size_t 
    { return djb2::compute(data, length); }

    static constexpr auto
    compute(std::string_view str)
    noexcept -> std::size_t 
    { return djb2::compute(str); }

    static constexpr auto
    compute(const char* str)
    noexcept -> std::size_t
    { return djb2::compute(str); }
};

struct murmur3_algorithm: AlgorithmBase<StringHashAlgo::murmur3> {
    static constexpr const char* name = "MURMUR3";
    static constexpr int quality_score = 6;
    
    static constexpr auto
    compute(const char* data, std::size_t length) 
    noexcept -> std::size_t 
    { return murmur3::compute(data, length); }

    static constexpr auto
    compute(std::string_view str) 
    noexcept -> std::size_t 
    { return murmur3::compute(str); }

    static constexpr auto
    compute(const char* str)
    noexcept -> std::size_t
    { return murmur3::compute(str); }
};


struct crc32_algorithm: AlgorithmBase<StringHashAlgo::crc32> {
    static constexpr const char* name = "CRC32";
    static constexpr int quality_score = 6;
    
    static constexpr auto
    compute(const char* data, std::size_t length) 
    noexcept -> std::size_t 
    { return crc32::compute(data, length); }

    static constexpr auto
    compute(std::string_view str) 
    noexcept -> std::size_t 
    { return crc32::compute(str); }

    static constexpr auto
    compute(const char* str)
    noexcept -> std::size_t
    { return crc32::compute(str); }
};

template <StringHashAlgo AlgorithmID> // TODO(ppqwqqq): Remove static_assert
struct AlgoSelector { static_assert(sizeof(AlgorithmID) == 0, "Unsupported algorithm"); };

template <>
struct AlgoSelector<StringHashAlgo::fnv1a> { using type = fnv1a_algorithm; };

template <>
struct AlgoSelector<StringHashAlgo::djb2> { using type = djb2_algorithm; };

template <>
struct AlgoSelector<StringHashAlgo::murmur3> { using type = murmur3_algorithm; };

template <>
struct AlgoSelector<StringHashAlgo::crc32> { using type = crc32_algorithm; };

template <HashAlgorithm Algorithm = fnv1a_algorithm>
class HashComputer {
public:
    using algorithm_type = Algorithm;

    static constexpr auto name() noexcept -> const char* { return Algorithm::name; }
    static constexpr auto algorithm_id() noexcept -> int { return Algorithm::algorithm_id; }

    static constexpr auto 
    compute(const char* data, std::size_t length) 
    noexcept -> std::size_t 
    { return Algorithm::compute(data, length); }

    static constexpr auto
    compute(std::string_view str)
    noexcept -> std::size_t
    { return Algorithm::compute(str); }

    static constexpr auto
    compute(const char* str) 
    noexcept -> std::size_t
    { return Algorithm::compute(str); }

    template <std::size_t N>
    static constexpr auto 
    compute(const char (&str)[N]) 
    noexcept -> std::size_t 
    { return Algorithm::compute(str,  N - 1); }

    constexpr auto 
    operator()(const char* data, size_t length) const 
    noexcept -> std::size_t 
    { return Algorithm::compute(data, length); }

    constexpr auto
    operator()(std::string_view str) const
    noexcept -> std::size_t 
    { return Algorithm::compute(str); }
};

class RuntimeHashComputer {
private:
    StringHashAlgo algorithm_;

public:
    explicit RuntimeHashComputer(StringHashAlgo algo): algorithm_(algo) { }

    auto compute(const char* data, std::size_t length) const 
    noexcept -> std::size_t {
        switch(algorithm_) {
            case StringHashAlgo::fnv1a:   return fnv1a_algorithm::compute(data, length);
            case StringHashAlgo::djb2:    return djb2_algorithm::compute(data, length);
            case StringHashAlgo::murmur3: return murmur3::compute(data, length);
            case StringHashAlgo::crc32:   return crc32::compute(data, length);
            default:                      return 0;
        }
    }

    [[nodiscard]]
    auto compute(std::string_view str) const noexcept -> std::size_t {
        return compute(str.data(), str.length());
    }
};

using DefaultHasher = HashComputer<fnv1a_algorithm>;
using FastHasher = HashComputer<djb2_algorithm>;
using QualityHasher = HashComputer<murmur3_algorithm>;
using QuickHasher = HashComputer<crc32_algorithm>;

template <StringHashAlgo AlgorithmID = StringHashAlgo::fnv1a>
constexpr auto 
compute_with_algorithm(const char* data, std::size_t lenghth) 
noexcept -> std::size_t {
    using SelectedAlogorithm = typename AlgoSelector<AlgorithmID>::type;
    return SelectedAlogorithm::compute(data, lenghth);
}

template <StringHashAlgo AlgorithmID = StringHashAlgo::fnv1a>
constexpr auto compute_with_algorithm(std::string_view str) noexcept -> std::size_t {
    using SelectedAlgorithm = typename AlgoSelector<AlgorithmID>::type;
    return SelectedAlgorithm::compute(str);
}

template <StringHashAlgo AlgorithmID = StringHashAlgo::fnv1a>
constexpr auto
compute_with_algorithm(const char* str)
noexcept -> std::size_t {
    using SelectedAlgorithm = typename AlgoSelector<AlgorithmID>::type;
    return SelectedAlgorithm::compute(str); 
}



} // namespace labelimg::core::refl::hash::algorithms
