#include <pch.h>
#include <chrono>
#include <cstdint>
#include <thread>

namespace labelimg::core::Asm {
// TODO(ppqwqqq): Add specific compile options 
class HighPrecisionTimer {
public:
    using cycle_count_t = std::uint64_t;

private:
    static inline double cpu_frequency_ghz = 0.0;
    static inline bool   frequence_calibrated = false;
    
    [[nodiscard]] static inline auto 
    get_cpu_cycles() // 获取 CPU 时钟周期数
    noexcept -> cycle_count_t {
#if defined (__x86_64__) || \
    defined (_M_X64)     || \
    defined (__i386)     || \
    defined (_M_IX86)

    // x86/x64 架构使用 RDTSC 指令
    cycle_count_t cycles;
    
    #if defined (__GNUC__) || defined (__clang__)
        std::uint32_t hi, lo;
        __asm__ volatile (
            "rdtsc"
            : "=a"(lo), "=d"(hi)
            :
            : "memory"
        );
        cycles = (static_cast<cycle_count_t>(hi) << 32) | lo;
    #elif defined (_MSC_VER)
        // TODO(ppqwqqq): Test on Windows
        cycles = __rdtsc();
    #else // 回退到标准库
        // TODO(ppqwqqq): Test fallback
        auto now = std::chrono::high_resolution_clock::now();
        cycles = static_cast<cycle_count_t>(now.time_since_epoch().count());
    #endif // defined (__GNUC__) || defined (__clang__)
    
#elif defined(__aarch64__) || defined(_M_ARM64)
    // ARM64 使用虚拟计数器
    cycle_count_t cycles;
    #if defined(__GNUC__) || defined(__clang__)
        __asm__ volatile (
            "mrs %0, cntvct_el0"
            : "=r"(cycles)
            : 
            : "memory"
        );
    #else // 回退到标准库
        auto now = std::chrono::high_resolution_clock::now();
        cycles = static_cast<cycle_count_t>(now.time_since_epoch().count());
    #endif

#elif defined(__arm__) || defined(_M_ARM_)
    // ARM32 架构
    cycle_count_t cycles;
    
    #if defined(__GNUC__) || defined(__clang__)
        __asm__ volatile (
            "mrc p15, 0, %0, c9, c13, 0"
            : "=r"(cycles)
            :
            : "memory"
        );
    #else // 回退到标准库
        auto now = std::chrono::high_resolution_clock::now();
        cycles = static_cast<cycle_count_t>(now.time_since_epoch().count());

    #endif
#else // 回退到标准库
    auto now = std::chrono::high_resolution_clock::now();
    cycles = static_cast<cycle_count_t>(now.time_since_epoch().count());
    
#endif // defined (__x86_64__) || defined (_M_X64) || defined (__i386) || defined (_M_IX86)
        return cycles;    
    }

    static void calibrate_frequency() {
        if (frequence_calibrated) return;
        
        constexpr int calibration_ms = 100;

        auto start_time = std::chrono::high_resolution_clock::now();
        auto start_cycles = get_cpu_cycles();

        std::this_thread::sleep_for(std::chrono::milliseconds(calibration_ms));
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto end_cycles = get_cpu_cycles();

        auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            end_time - start_time
        ).count();

        auto cycle_diff = end_cycles - start_cycles;
        
        cpu_frequency_ghz = static_cast<double>(cycle_diff) / duration_ns;
        frequence_calibrated = true;
    }
    
};


} // namespace labelimg::core::Asm