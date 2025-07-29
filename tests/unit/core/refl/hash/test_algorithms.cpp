// ---------------Reflection.Hash.Algorithms--------------- //
//         
//     Description: 
//          Test implemented algorithms for computing hash
//  
//     Algorithms: 
//        1. FNV-1a
//        2. DJB2
//        3. MURMUR3
//        4. CRC32
//
//
//
// -------------------------------------------------------- //

// TODO(ppqwqqq): Replace the below common headers with pch
#include <gtest/gtest.h>
#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
#include <array>

#include <core/refl/detail/hash/hash_algorithms.hpp>
#include "core/refl/detail/hash.hpp"

using namespace labelimg::core::refl::hash::algorithms;

class HashAlgorithmTest: public ::testing::Test {
protected:
    void SetUp() override {
        // Multiple types of string
        test_strings = {
            // common types
            "",
            "a",
            "abc",
            "Hello, world",
            "The another day will be nice.",
            // unusual types
            std::string(1000, 'x'), // long string
            std::string(1, '\0'),   // empty string
            "01239801208757238478345",   // numeric string 
            "!@#(*((><>?:{})))",         // special string
        };
    }

    std::vector<std::string> test_strings;

    // testing known string (Optional: pre-computed) 
    static constexpr std::string_view known_string = "Hello, world!";
};

// FNV-1a 
class FNV1aTest: public HashAlgorithmTest {
protected:
    static constexpr std::size_t expected_hello_world_hash 
        = fnv1a::compute("Hello, world!", 13);
};

TEST_F(FNV1aTest, BasicFunctionality) {
    // test empty string
    EXPECT_EQ(
        fnv1a::compute("", 0),
        fnv1a::default_constants::offset_basis
    );

    // const char* 
    const char* test_str = "test";
    auto hash1 = fnv1a::compute(test_str, 4);

    // std::string 
    std::string_view sv(test_str);
    auto hash2 = fnv1a::compute(sv);

    EXPECT_EQ(hash1, hash2);
}

TEST_F(FNV1aTest, CompileTimeConsistency) {
    // compile-time's result == runtime's result (consistent result)
    constexpr auto compile_time_hash 
        = fnv1a::compute("compile_test", 12);
    
    std::string runtime_str = "compile_test";
    auto runtime_hash 
        = fnv1a::compute(runtime_str.data(), runtime_str.length());
    
    EXPECT_EQ(
        compile_time_hash,
        runtime_hash
    );
}

TEST_F(FNV1aTest, DifferentInputSameAlgo) {
    // different hash 
    std::set<std::size_t> hashes;

    for (const auto& str: test_strings) {
        auto hash = fnv1a::compute(str.data(), str.length());
        
        // detect hash collision
        EXPECT_TRUE(hashes.insert(hash).second) 
            << "Coliision detected for string: " << str << "'";
    }    
}

// DJB2
class DJB2Test: public HashAlgorithmTest { };

TEST_F(DJB2Test, BasicFunctionality) {
    // test empty string
    EXPECT_EQ(djb2::compute("", 0), djb2::initial_value);

    // const char*
    const char* test_str = "djb2_test";
    auto hash1 = djb2::compute(test_str, 9);

    // std::string_view
    std::string_view sv(test_str);
    auto hash2 = djb2::compute(sv);

    EXPECT_EQ(
        hash1,
        hash2
    );
}

TEST_F(DJB2Test, XorVariant) {
    // test DJB2 variant 
    const char* test_str = "test_xor";
    auto hash_add = djb2::compute(test_str, 8);
    auto hash_xor = djb2::compute_xor(test_str, 8);
    
    EXPECT_NE(
        hash_add,
        hash_xor
    );
}

// Murmur3
class MurmurHash3Test: public HashAlgorithmTest { };

TEST_F(MurmurHash3Test, BasicFunctionality) {
    const char* test_str = "murmur_test";
    auto hash1 = murmur3::compute(test_str, 11, 0);  // seed: 0
    auto hash2 = murmur3::compute(test_str, 11, 42); // seed: 42
    
    EXPECT_NE( // different seeds produce different hash
        hash1,
        hash2
    );
}

TEST_F(MurmurHash3Test, SeedSensitivity) {
    std::string_view test_str = "seed_test";
    std::vector<std::size_t> hashes;
    // different seeds produce different hash
    for (std::uint32_t seed = 0; seed < 10; ++seed) {
        auto hash = murmur3::compute(test_str.data(), test_str.length(), seed);
        hashes.push_back(hash);
    }

    std::sort(hashes.begin(), hashes.end());
    auto unique_end = std::unique(hashes.begin(), hashes.end());
    EXPECT_EQ(unique_end, hashes.end()) 
        << "Seeds should produce different hashes";
}

// CRC32
class CRC32Test: public HashAlgorithmTest {};

TEST_F(CRC32Test, BasicFunctionality) {
    // test known hash
    const char* test_str = "123456789";
    auto hash = crc32::compute(test_str, 9);

    constexpr std::uint32_t expected = 0xCBF43926; // CRC32("123456789")
    EXPECT_EQ(
        hash,
        expected
    );
}

class AlgorithmSelectorTest: public ::testing::Test {};

TEST_F(AlgorithmSelectorTest, TemplateSelection) {
    constexpr std::string_view test_str = "selector_test";
    
    auto fnv_hash = compute_with_algorithm<StringHashAlgo::fnv1a>(test_str);
    auto djb_hash = compute_with_algorithm<StringHashAlgo::djb2>(test_str);
    auto murmur_hash = compute_with_algorithm<StringHashAlgo::murmur3>(test_str);
    auto crc_hash = compute_with_algorithm<StringHashAlgo::crc32>(test_str);

    // different algorithms produce different hash
    EXPECT_NE(fnv_hash, djb_hash);
    EXPECT_NE(fnv_hash, murmur_hash);
    EXPECT_NE(fnv_hash, crc_hash);
    EXPECT_NE(djb_hash, murmur_hash);
    EXPECT_NE(djb_hash, crc_hash);
    EXPECT_NE(murmur_hash, crc_hash);
}

class UnifiedInterfaceTest: public ::testing::Test {};

TEST_F(UnifiedInterfaceTest, HashComputer) {
    using fnv_computer = HashComputer<fnv1a_algorithm>;
    using djb_computer = HashComputer<djb2_algorithm>;
    using murmur_computer = HashComputer<murmur3_algorithm>;
    using crc_computer = HashComputer<crc32_algorithm>;

    constexpr auto fnv_hasher = fnv_computer{};
    constexpr auto djb_hasher = djb_computer{};
    constexpr auto murmur_hasher = murmur_computer{};
    constexpr auto crc_hasher = crc_computer{};

    constexpr std::string_view test_str = "interface_test";

    auto fnv_hash = fnv_hasher(test_str);
    auto djb_hash = djb_hasher(test_str);
    auto murmur_hash = murmur_hasher(test_str);
    auto crc_hash = crc_hasher(test_str);

    // different algorithms produce different hash
    EXPECT_NE(fnv_hash, djb_hash);
    EXPECT_NE(fnv_hash, murmur_hash);
    EXPECT_NE(fnv_hash, crc_hash);
    EXPECT_NE(djb_hash, murmur_hash);
    EXPECT_NE(djb_hash, crc_hash);
    EXPECT_NE(murmur_hash, crc_hash);

    // validate computer's name
    EXPECT_STREQ(fnv_computer::name(), "FNV-1a");
    EXPECT_STREQ(djb_computer::name(), "DJB2");
    EXPECT_STREQ(murmur_computer::name(), "MURMUR3");
    EXPECT_STREQ(crc_computer::name(), "CRC32");
}
