#ifndef HP_TIMER_HPP
#define HP_TIMER_HPP
#include <pch.h>

namespace labelimg::core::asm_ {
// TODO(ppqwqqq): Add specific compile options 
class HighPrecisionTimer {
public:
    using cycle_count_t = std::uint64_t;

private:
    static inline double cpu_frequency_ghz = 0.0;
    static inline bool   frequency_calibarted = false;
    
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
        if (frequency_calibarted) return;
        
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
        frequency_calibarted = true;
    }
public:
    // 高精度时间点
    struct TimePoint {
        cycle_count_t cycles;
        std::chrono::high_resolution_clock::time_point wall_time;
        
        TimePoint()
            : cycles{get_cpu_cycles()}
            , wall_time{std::chrono::high_resolution_clock::now()} 
            { }

        explicit TimePoint(cycle_count_t c)
            : cycles{c}
            , wall_time{std::chrono::high_resolution_clock::now()}
            { }
    };

    // 高精度时间间隔
    struct Duration {
        cycle_count_t cycles;
        std::chrono::nanoseconds wall_duration;

        Duration(const TimePoint& start, const TimePoint& end)
            : cycles{end.cycles - start.cycles}
            , wall_duration(std::chrono::duration_cast<std::chrono::nanoseconds>(
                end.wall_time - start.wall_time
            )) {}
        
        // 将周期转换为 纳秒
        [[nodiscard]] auto to_nanoseconds() const -> double {
            if (!frequency_calibarted) { }
                // TODO(ppqwqqq): Need to calibrate frequency
            
            if (cpu_frequency_ghz > 0.0) 
                return static_cast<double>(cycles) / cpu_frequency_ghz;
            else 
                return static_cast<double>(wall_duration.count());
        
        }

        // 将周期转换为 微秒
        [[nodiscard]] auto to_microseconds() const -> double {
            return to_nanoseconds() / 1e3;
        }

        // 将周期转换 毫秒
        [[nodiscard]] auto to_millisecons() const -> double {
            return to_nanoseconds() / 1e6;
        }

        // 将周期转换为 秒
        [[nodiscard]] auto to_seconds() const -> double {
            return to_nanoseconds() / 1e9;
        }

        // 获取 CPU 周期数
        [[nodiscard]] auto get_cycles() const -> cycle_count_t {
            return cycles;
        }

        // 获取墙钟时间
        [[nodiscard]] auto get_wall_duration() const -> std::chrono::nanoseconds {
            return wall_duration;
        }
    };

    [[nodiscard]] static auto now() -> TimePoint {
        return TimePoint{};
    }

    [[nodiscard]] static auto get_cpu_frequency() -> double {
        if (!frequency_calibarted) calibrate_frequency();
        return cpu_frequency_ghz;
    }

    // 手动（重新）校准频率
    static void recalibrate() {
        frequency_calibarted = false;
        calibrate_frequency();
    }
};


} // namespace labelimg::core::asm_ 

#endif // HP_TIMER_HPP