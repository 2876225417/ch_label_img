// ---------------Reflection.Hash.Performance--------------- //
//         
//                      Benchmark
//
//     Description: 
//          Test performance for computing hash
//  
//     Algorithms: 
//        1. FNV-1a
//        2. DJB2
//        3. MURMUR3
//        4. CRC32
//
//     Target:
//        Performance Test    
//
// -------------------------------------------------------- //

#include <benchmark/benchmark.h>
#include <benchmark_utils.hpp>
#include <sample_data.hpp>

#include <string>
#include <vector>
#include <functional>

#include <core/refl/detail/hash/hash_algorithms.hpp>

using namespace labelimg::core::refl::hash::algorithms;


using namespace test::common;
using namespace test::common;

template <StringHashAlgo Algorithm>
static void BM_SingleStringHashFor(benchmark::State& state) {
    const std::string test_str = "benchmark_string";

    for (auto _: state) {
        auto result = compute_with_algorithm<StringHashAlgo::fnv1a>(test_str);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(BM_SingleStringHashFor, StringHashAlgo::fnv1a) -> Name("Hash/fnv1a");
BENCHMARK_TEMPLATE(BM_SingleStringHashFor, StringHashAlgo::djb2) -> Name("Hash/djb2");
BENCHMARK_TEMPLATE(BM_SingleStringHashFor, StringHashAlgo::murmur3) -> Name("Hash/murmur3");
BENCHMARK_TEMPLATE(BM_SingleStringHashFor, StringHashAlgo::crc32) -> Name("Hash/crc32");




template <StringHashAlgo Algorithm>
static void BM_RuntimeSingleStringHashFor(benchmark::State& state) {
    const std::string test_str = "benchmark_string";
    RuntimeHashComputer runtime_hasher(Algorithm);

    for (auto _: state) {
        auto result = runtime_hasher.compute(test_str);
        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_TEMPLATE(BM_RuntimeSingleStringHashFor, StringHashAlgo::fnv1a) -> Name("Hash/fnv1a(runtime)");
BENCHMARK_TEMPLATE(BM_RuntimeSingleStringHashFor, StringHashAlgo::djb2) -> Name("Hash/djb2(runtime)");
BENCHMARK_TEMPLATE(BM_RuntimeSingleStringHashFor, StringHashAlgo::murmur3) -> Name("Hash/murmur3(runtime)");
BENCHMARK_TEMPLATE(BM_RuntimeSingleStringHashFor, StringHashAlgo::crc32) -> Name("Hash/crc32(runtime)");

static void BM_BatchStringHash(benchmark::State& state) {
    
}