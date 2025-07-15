#pragma once

#include "utils/singleton.h"
#include <bits/c++config.h>

# if (__cplusplus >= 202002L) && (__has_builtin(__builtin_bit_cast))
#include <bit>
#define HAS_BIT_CAST 1
#else
#define HAS_BIT_CAST 0
#endif




#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
namespace labelimg::core::refl::hash {

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


constexpr auto // 哈希组合 
hash_combine(std::size_t lhs, std::size_t rhs)
noexcept -> std::size_t {
    return // 黄金比例常数的近似值 
        lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
}

template <typename... Args>
constexpr auto // 多个哈希组合
hash_combine(std::size_t first, Args... args) 
noexcept -> std::size_t {
    if constexpr (sizeof...(args) == 0) return first;
    else return hash_combine(first, hash_combine(args...));
}

constexpr auto // 编译期字符串哈希计算 
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

constexpr auto // 字符串字面量 哈希
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

constexpr auto // std::string_view 哈希
string_hash(std::string_view str) 
noexcept -> std::size_t {
    return string_hash(str.data(), str.length());
}

template <typename T> constexpr auto 
type_name_hash() // 类型名称哈希
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
type_address_hash() // 类型地址哈希
noexcept -> std::size_t {       // 根据类型地址计算哈希(运行时)
    static char type_id = 0;    // 使用类型的静态变量地址作为唯一标识符
    return reinterpret_cast<std::size_t>(&type_id);
}

template <typename T> constexpr auto
type_hash() // 编译期 类型哈希
noexcept -> std::size_t {
    if constexpr (requires { type_name_hash<T>(); }) {
        return type_name_hash<T>();
    } else {
        // 编译期不可用
        // 调用 runtime_type_hash()
        return 0;   // 默认值: 0
    }
}

template <typename T> inline auto 
runtime_type_hash() // 运行时 类型哈希
noexcept -> std::size_t {
    return type_address_hash<T>();
}

template <typename... Types>
constexpr auto // 递归 组合类型哈希
combine_type_hashes()
noexcept -> std::size_t {
    if constexpr (sizeof...(Types) == 0) return 0;
    else return hash_combine(type_hash<Types>()...);
}

template <typename T>
constexpr auto // 浮点型 哈希
float_hash(T value)
noexcept -> std::size_t {
    static_assert(std::is_floating_point_v<T>);
#ifdef HAS_BIT_CAST
    if constexpr (sizeof(T) <= sizeof(std::size_t)) {
        if      constexpr (sizeof(T) == sizeof(std::uint32_t)) 
        {
            return static_cast<std::size_t>(
                std::bit_cast<std::uint32_t>(static_cast<float>(value))
            );
        } 
        else if constexpr (sizeof(T) == sizeof(std::uint64_t))
        {
            return std::bit_cast<std::uint64_t>(
                static_cast<double>(value)
            );
        }
        else
        {   return type_hash<T>(); }
    } else{ return type_hash<T>(); }
#else
    if (std::is_constant_evaluated())  
    {   return type_hash<T>(); } // 编译期只返回类型哈希
    else 
    {
        if constexpr (sizeof(T) <= sizeof(std::size_t))
        {
=            std::size_t result = 0;
            std::memcpy(&result, &value, sizeof(value));
            return result;
        } 
        else
        {
            const auto* bytes = reinterpret_cast<const unsigned char*>(&value);
            std::size_t result =  type_hash<T>();
            for (std::size_t i = 0; i < sizeof(T); ++i) 
            { result = hash_combine(result, static_cast<std::size_t>(bytes[i])); }
            return result;
        }
    }                           // 运行时使用完整的位级哈希
#endif // HAS_BIT_CAST
}

template <typename T>
auto runtime_float_hash(T value) 
noexcept -> std::size_t {
    static_assert(std::is_floating_point_v<T>);
    if constexpr (sizeof(T) <= sizeof(std::size_t)) {
        std::size_t result = 0;
        std::memcpy(&result, &value, sizeof(value));
        return result;
    } else {
        const auto* bytes = reinterpret_cast<const unsigned char*>(&value);
        std::size_t result = type_hash<T>();
        for (std::size_t i = 0; i < sizeof(T); ++i) 
        { result = hash_combine(result, static_cast<std::size_t>(bytes[i])); }
        return result;
    }
}

template <typename T>
constexpr auto
pointer_type_hash() 
noexcept -> std::size_t 
{ return hash_combine(type_hash<T>(), string_hash("*")); }

template <typename T>
constexpr auto
reference_type_hash()
noexcept -> std::size_t 
{ return hash_combine(type_hash<T>(), string_hash("&")); }

template <typename T>
constexpr auto
rvalue_reference_type_hash() 
noexcept -> std::size_t { return hash_combine(type_hash<T>(), string_hash("&&")); }

template <typename T, std::size_t N>
constexpr auto
array_type_hash()
noexcept -> std::size_t { return hash_combine(type_hash<T>(), N); }

template <typename T>
constexpr auto
const_type_hash()
noexcept -> std::size_t { return hash_combine(type_hash<T>(), string_hash("const")); }

template <typename T>
constexpr auto
volatile_type_hash()
noexcept -> std::size_t { return hash_combine(type_hash<T>(), string_hash("volatile")); }

// pair traits
template <typename T>
struct is_pair: std::false_type {};
template <typename T1, typename T2>
struct is_pair<std::pair<T1, T2>>: std::true_type {};
template <typename T>
constexpr bool is_pair_v = is_pair<T>::value;

// tuple traits
template <typename T>
struct is_tuple: std::false_type {};
template <typename... Args>
struct is_tuple<std::tuple<Args...>>: std::true_type {};
template <typename T>
constexpr bool is_tuple_v = is_tuple<T>::value;

// HASHable
template <typename T, typename = void>
struct is_hashable: std::false_type {};
template <typename T>
struct is_hashable<
    T, 
    std::void_t<decltype(std::hash<T>{}(std::declval<T>()))>
>: std::true_type {};
template <typename T>
constexpr bool is_hashable_v = is_hashable<T>::value;

// Tuple Hash Helper
template <typename Tuple, std::size_t... Is>
constexpr auto hash_tuple(const Tuple& t, std::index_sequence<Is...>)
noexcept -> std::size_t 
{ return hash_combine(compute_hash(std::get<Is>(t))...); }

template <typename T> 
constexpr auto compute_hash(T&& value)
noexcept -> std::size_t {
    using DecayedT = std::decay_t<T>;
    using RawT = std::remove_cv_t<std::remove_reference_t<T>>;

    if      constexpr (std::is_same_v<DecayedT, std::size_t>) 
    {   return value; }                               // size_t
    else if constexpr (std::is_integral_v<DecayedT>) 
    {   return static_cast<size_t>(value); }          // int
    else if constexpr (std::is_convertible_v<DecayedT, std::string_view>)
    {   return string_hash(std::forward<T>(value)); } // string
    else if constexpr (std::is_pointer_v<RawT> && 
                      !std::is_convertible_v<DecayedT, std::string_view>)
    { 
        using PointeeType = std::remove_pointer_t<RawT>;
        return HASH_COMBINE(
            pointer_type_hash<PointeeType>(),
            reinterpret_cast<std::uintptr_t>(value)
        );
    }                                                 // pointer
    else if constexpr (std::is_enum_v<DecayedT>)
    {
        return static_cast<std::size_t>(
            static_cast<std::underlying_type_t<DecayedT>>(value)
        );
    }                                                 // enum 
    else if constexpr (std::is_floating_point_v<DecayedT>) 
    {
        if (std::is_constant_evaluated()) 
        {   
            return float_hash(value);
        } 
        else {    // 对于大类型(如 long double) 使用 hash combine
            return runtime_float_hash(value);
        }
    }                                                 // floating
    else if constexpr (is_pair_v<DecayedT>)
    {
        return hash_combine(
            compute_hash(value.first),
            compute_hash(value.second)
        );
    }                                                 // std::pair
    else if constexpr (is_tuple_v<DecayedT>)
    {
        return hash_tuple(
            value, 
            std::make_index_sequence<std::tuple_size_v<DecayedT>>{}
        );
    }                                                 // std::tuple
    else if constexpr (is_hashable_v<DecayedT>)
    {   return std::hash<DecayedT>{}(value); }        // Hashable
    else 
    {
        static_assert(
            sizeof(T) == 0,
            "Type not supported for hashing. Consider adding std::hash specialization or convertion"
        );
        return 0;
    }                                                 // Default
}

template <typename T>
constexpr auto REFL_STRING_HASH(T&& str) 
noexcept -> std::size_t
{ return REFL_STRING_HASH(std::forward<T>(str), std::size_t{}); }

template <typename T>
constexpr auto REFL_STRING_HASH(T&& str, std::size_t length)
noexcept -> std::size_t {
    using DecayedT = std::decay_t<T>;

    if constexpr (std::is_same_v<DecayedT, const char*> ||
                  std::is_same_v<DecayedT, char*>) {
        if (length == 0) return string_hash(str);
        else             return string_hash(str, length);
    } else if constexpr (std::is_convertible_v<T, std::string_view>) {
        std::string_view sv{std::forward<T>(str)};
        return string_hash(
             sv.data(),
          length == 0 ? sv.length() : std::min(length, sv.length())
        );
    }
}

template <typename T>
constexpr auto REFL_TYPE_HASH() 
noexcept -> std::size_t
{ return type_hash<T>(); }

template <typename T>
constexpr auto REFL_FLOATING_POINT_HASH(float f)
noexcept -> std::size_t 
{ return float_hash(f); }

template <typename... Args>
constexpr auto HASH_COMBINE(Args... args)
noexcept -> std::size_t 
{ return hash_combine(static_cast<std::size_t>(args)...); }

template <typename... Types>
constexpr auto COMBINE_TYPE_HASHES()
noexcept -> std::size_t
{ return combine_type_hashes<Types...>(); }

template <typename... Args>
constexpr auto MIXED_HASH(Args&&... args)
noexcept -> std::size_t 
{ return hash_combine(compute_hash(std::forward<Args>(args))...); }

template <typename T>
constexpr auto REFL_POINTER_TYPE_HASH()
noexcept -> std::size_t 
{ return pointer_type_hash<T>(); }

template <typename T>
constexpr auto REFL_REFERENCE_TYPE_HASH()
noexcept -> std::size_t 
{ return reference_type_hash<T>(); }

template <typename T>
constexpr auto REFL_RVALUE_REFERENCE_TYPE_HASH()
noexcept -> std::size_t 
{ return rvalue_reference_type_hash<T>(); }

template <typename T, std::size_t N>
constexpr auto REFL_ARRAY_TYPE_HASH()
noexcept -> std::size_t 
{ return array_type_hash<T, N>(); }

template <typename T>
constexpr auto REFL_CONST_TYPE_HASH()
noexcept -> std::size_t 
{ return const_type_hash<T>(); }

template <typename T>
constexpr auto REFL_VOLATILE_TYPE_HASH()
noexcept -> std::size_t 
{ return volatile_type_hash<T>(); }

#ifdef NDEBUG
class HashCollisionDetector: // 哈希冲突检测器 
    public Singleton<HashCollisionDetector> {
    MAKE_SINGLETON_NO_DEFAULT_CTOR_DTOR(HashCollisionDetector)
public:
    auto check_and_record(std::size_t hash, const char* type_name) {
        if (_hashes.find(hash) != _hashes.end()) {
            std::cerr << "Hash collision detected for type: " << type_name 
                      << " (hash: " << hash << ")\n";
            return false;
        }
        _hashes.insert(hash);
        return true;
    }
private:
    std::unordered_set<std::size_t> _hashes;
};

template <typename HashType>
constexpr auto 
REFL_CHECK_HASH_COLLISION(HashType hash, const char* type_name)
noexcept -> bool { return labelimg::core::refl::detail::HashCollisionDetector::instance().check_and_record(hash, type_name); }

#else
template <typename HashType>
constexpr auto 
REFL_CHECK_HASH_COLLISION(HashType hash, const char* type_name) 
noexcept -> bool { return true; }
#endif // NDEBUG


}  // namespace labelimg::core::refl::detail
