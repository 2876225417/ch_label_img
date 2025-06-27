#ifndef ASYNC_LOGGER_H
#define ASYNC_LOGGER_H

#include "utils/singleton.h"
#include <atomic>
#include <chrono>
#include <core/message_queue.hpp>
#include <csignal>
#include <cstddef>
#include <memory>
#include <mutex>
#include <ostream>
#include <thread>
#include "core/logger.hpp"
namespace labelimg::core::logger {

class LogStream: private NonCopyable  {
public:
    LogStream();
    ~LogStream();

    template <typename T>
    auto operator<<(const T& value) -> LogStream& {
        m_oss << value;
        return *this;
    }
private:
    std::ostringstream m_oss;
};

#define async_log LogStream()
namespace v1 {

class AsyncLogger: private Singleton<AsyncLogger> {
    MAKE_SINGLETON_NOT_DEFAULT_CTOR_DTOR(AsyncLogger)
public:
    static auto instance() -> AsyncLogger&;
    void log(std::string);
private:
    void worker_thread_func();

    class Impl;
    std::unique_ptr<Impl> pimpl;
};
} // namespace v1 

inline namespace v2 {

template <typename Derived>
class AsyncLoggerApi {
    void log(std::string message) {
        static_cast<Derived*>(this)->log_impl(std::move(message));
    }

    void stop() {
        static_cast<Derived*>(this)->stop_impl();
    }

protected:
    ~AsyncLoggerApi() = default;
};

template <typename ConcurrencyPolicy = queue::MutexPolicy>
class AsyncLogger;

template <>
class AsyncLogger<queue::MutexPolicy>: public AsyncLoggerApi<AsyncLogger<queue::MutexPolicy>>
                                     , public Singleton<AsyncLogger<queue::MutexPolicy>> {
    MAKE_SINGLETON_NO_DEFAULT_CTOR_DTOR(AsyncLogger<queue::MutexPolicy>)
public:
    void log_impl(std::string message);
    void stop_impl();
private:
    AsyncLogger();
    ~AsyncLogger();

    void worker_thread_func();

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

class AsyncLogger<queue::MutexPolicy>::Impl {
public:
    Impl(): m_done {false} {
        m_worker = std::thread{&AsyncLogger::Impl::worker_thread_func, this};
    }

    ~Impl() { stop(); }

    void log(std::string message) {
        m_queue.push(std::move(message));
    }
    
    void stop() {
        if (!m_done.exchange(true)) {
            m_queue.push("");
            if (m_worker.joinable()) m_worker.join();
        }
    }

private:
    void worker_thread_func() {
        while (!m_done) {
            std::string message;
            
            if (m_queue.wait_for_pop(message, std::chrono::milliseconds(100))) {
                if (m_done && message.empty()) break;
                std::cout << message << std::flush;
            }
        }
        std::string message;
        while (m_queue.try_pop(message)) {
            if (!message.empty()) std::cout << message << std::flush;
        }
    }

    std::atomic<bool> m_done;
    queue::MessageQueue<std::string, queue::MutexPolicy> m_queue;
    std::thread m_worker;
};

inline AsyncLogger<queue::MutexPolicy>::AsyncLogger(): pImpl(std::make_unique<Impl>()) {} 

inline AsyncLogger<queue::MutexPolicy>::~AsyncLogger() = default;

inline void AsyncLogger<queue::MutexPolicy>::log_impl(std::string message) {
    pImpl->log(std::move(message));
}

inline void AsyncLogger<queue::MutexPolicy>::stop_impl() {
    pImpl->stop();
}

template <>
class AsyncLogger<queue::CoroutinePolicy>: public AsyncLoggerApi<AsyncLogger<queue::CoroutinePolicy>>
                                         , public Singleton<AsyncLogger<queue::CoroutinePolicy>> {
    MAKE_SINGLETON_NO_DEFAULT_CTOR_DTOR(AsyncLogger<queue::CoroutinePolicy>)
public:
    void log_impl(std::string message);
    void stop_impl();                                
    void run_util_complete();
private:
    AsyncLogger();
    ~AsyncLogger();

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

class AsyncLogger<queue::CoroutinePolicy>::Impl {
public:
    Impl(): m_done{false}
          , m_worker_task(worker_coroutine())
          { }

    ~Impl() { stop(); }

    void log(std::string message) {
        m_queue.push(std::move(message));
    }

    void stop() {
        if (!m_done.exchange(true)) m_queue.push("");
    }

    void run_until_complete() {
        while (!m_worker_task.is_ready()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    auto worker_coroutine() -> queue::Task<void> {
        while (!m_done) {
            try {                                      
                std::string message = co_await m_queue.async_pop();

                if (m_done && message.empty()) break;

                if (!message.empty()) std::cout << message << std::flush;
            } catch (const std::exception& e) {
                std::cerr << "Logger coroutine error: " << e.what() << '\n';
            }
        }
        
        while (auto msg = m_queue.try_pop()) {
            if (msg->empty()) std::cout << *msg << std::flush;
        }

        co_return;
    }
private:
    std::atomic<bool> m_done;
    queue::MessageQueue<std::string, queue::CoroutinePolicy> m_queue;
    queue::Task<void> m_worker_task; 
};

inline AsyncLogger<queue::CoroutinePolicy>::AsyncLogger()
    : pImpl(std::make_unique<Impl>()) {}

inline AsyncLogger<queue::CoroutinePolicy>::~AsyncLogger() {}

inline void 
AsyncLogger<queue::CoroutinePolicy>::log_impl(std::string message) {
    pImpl->log(std::move(message));
}

inline void 
AsyncLogger<queue::CoroutinePolicy>::stop_impl() {
    pImpl->stop();
}

inline void AsyncLogger<queue::CoroutinePolicy>::run_util_complete() {
    pImpl->run_until_complete();
}

using MutexAsyncLogger = AsyncLogger<queue::MutexPolicy>;
using CoroutineAsncLogger = AsyncLogger<queue::CoroutinePolicy>;

template <typename T>
concept AsyncLoggerConcept = requires(T logger, std::string message) {
    { logger.log_impl(std::move(message)) } -> std::same_as<void>;
    { logger.stop_impl() } -> std::same_as<void>;
    { T::instance() } -> std::same_as<T&>;
};

template <typename T>
concept BatchProcessorConcept = requires(T processor, std::vector<std::string> messages) {
    { processor.process_batch(std::move(messages)) } -> std::same_as<void>;
    { processor.should_flush() } -> std::convertible_to<bool>;
    { processor.get_batch_size() } -> std::convertible_to<size_t>;
};

template <typename T>
concept FormatterConcept = requires(T formmater, std::string_view message) {
    { formmater.format(message) } -> std::convertible_to<std::string>;
};

struct ImmediateStrategy {
    template <AsyncLoggerConcept Logger>
    void process(Logger& logger, std::string message) {
        logger.log_impl(std::move(message));
    }
    
    template <AsyncLoggerConcept Logger>
    [[maybe_unused]] void flush(Logger& logger) { }

    [[nodiscard]] constexpr auto
    needs_flush() const -> bool { return false; }
};

template <size_t BatchSize = 100, size_t FlushIntervalMs = 50>
struct BatchStrategy {
    std::vector<std::string> batch;
    std::chrono::steady_clock::time_point last_flush;

    BatchStrategy(): last_flush{std::chrono::steady_clock::now()} {
        batch.reserve(BatchSize);
    }

    template <AsyncLoggerConcept Logger>
    void process(Logger& logger, std::string message) {
        batch.push_back(std::move(message));
        
        if (should_flush()) flush(logger);
    }

    template <AsyncLoggerConcept Logger>
    void flush(Logger& logger) {
        if (!batch.empty()) {
            std::string combined;
            // for (const auto& msg: batch) combined += msg;
            combined = std::accumulate(batch.begin(), batch.end(), std::string{});
            logger.log_impl(std::move(combined));
            batch.clear();
        }
        last_flush = std::chrono::steady_clock::now();
    }

    [[nodiscard]] auto 
    should_flush() const -> bool {
        return batch.size() >= BatchSize || 
               std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - last_flush
               ).count() >= FlushIntervalMs;
    }

    [[nodiscard]] constexpr auto needs_flush() -> bool { return true; }
    [[nodiscard]] constexpr auto get_batch_size() const -> size_t { return BatchSize; } 
};


template <auto... Values>
struct ValueList {};

template <typename... Types>
struct TypeList {};

template <typename IntValues, typename TypeValues>
struct BatchLoggerTraits {};

template <size_t BatchSize, size_t FlushIntervalMs, typename BackendPolicy>
struct BatchLoggerTraits<ValueList<BatchSize, FlushIntervalMs>, TypeList<BackendPolicy>> {
    static constexpr size_t BatchSize_ = BatchSize;
    static constexpr size_t FlushIntervalMs_ = FlushIntervalMs;
    using BackendPolicy_ = BackendPolicy;
};


enum BatchType: std::int8_t {
    Default,    // 100, 50
    Fast,       // 50, 25
    Large,      // 500, 200
    Realtime    // 10, 10
};

enum BackendType: std::int8_t {
    Mutex,     // std::mutex
    Coroutine   // std::coroutine
};

template <BatchType Type>
struct BatchParams;

template <>
struct BatchParams<BatchType::Default> {
    static constexpr size_t batch_size = 100;
    static constexpr size_t flush_interval_ms = 50;
};

template <>
struct BatchParams<BatchType::Fast> {
    static constexpr size_t batch_size = 50;
    static constexpr size_t flush_interval_ms = 25;    
};

template <>
struct BatchParams<BatchType::Large> {
    static constexpr size_t batch_size = 500;
    static constexpr size_t flush_interval_ms = 200;
};

template <>
struct BatchParams<BatchType::Realtime> {
    static constexpr size_t batch_size = 10;
    static constexpr size_t flush_interval_ms = 10;
};

template <BackendType Type>
struct BackendPolicy;

template <>
struct BackendPolicy<BackendType::Mutex> {
    using type = queue::MutexPolicy;
};

template <>
struct BackendPolicy<BackendType::Coroutine> {
    using type = queue::CoroutinePolicy;
};

template <BackendType Backend>
using BackendPolicy_t = typename BackendPolicy<Backend>::type;


template <BatchType Batch, BackendType Backend>
using BatchConfig = BatchLoggerTraits<
            ValueList<BatchParams<Batch>::batch_size, BatchParams<Batch>::flush_interval_ms>, 
            TypeList<BackendPolicy_t<Backend>>
>;

using DefaultMutexBatch = BatchConfig<BatchType::Default, BackendType::Mutex>;
using DefaultCoroutineBatch = BatchConfig<BatchType::Default, BackendType::Coroutine>;

using FastMutexBatch = BatchConfig<BatchType::Fast, BackendType::Mutex>;
using FastCoroutineBatch = BatchConfig<BatchType::Fast, BackendType::Coroutine>;

using LargeMutexBatch = BatchConfig<BatchType::Large, BackendType::Mutex>;
using LargeCoroutineBatch = BatchConfig<BatchType::Large, BackendType::Coroutine>;

using RealtimeMutexBatch = BatchConfig<BatchType::Realtime, BackendType::Mutex>;
using RealtimeCoroutineBatch = BatchConfig<BatchType::Realtime, BackendType::Coroutine>;

template <size_t BatchSize = 100, size_t FlushIntervalMs = 50>
using ManualMutexBatch = BatchLoggerTraits<ValueList<BatchSize, FlushIntervalMs>, TypeList<queue::MutexPolicy>>;

template <size_t BatchSize = 100, size_t FlushIntervalMs = 50>
using ManualCoroutineBatch = BatchLoggerTraits<ValueList<BatchSize, FlushIntervalMs>, TypeList<queue::CoroutinePolicy>>;


template <typename Traits = DefaultMutexBatch>
class BatchAsyncLogger: public AsyncLoggerApi<BatchAsyncLogger<Traits>>
                      , public Singleton<BatchAsyncLogger<Traits>> {
    MAKE_SINGLETON_NO_DEFAULT_CTOR_DTOR(BatchAsyncLogger)
public:
    void log_impl(std::string message);
    void stop_impl();

    void flush();

    [[nodiscard]] auto get_batch_size() const -> size_t; 
    [[nodiscard]] auto get_pending_count() const -> size_t;
private:
    BatchAsyncLogger();
    ~BatchAsyncLogger();

    class Impl;
    std::unique_ptr<Impl> pImpl;
};

template <typename Traits>
class BatchAsyncLogger<Traits>::Impl {
public:
    Impl(): m_done{false}, m_strategy{} {
        m_worker = std::thread{&BatchAsyncLogger::Impl::worker_thread_func, this};
    }

    ~Impl() {
        stop();
    }

    void stop() {
        if (!m_done.exchange(true)) {
            flush_internal();
            m_backend_logger.stop_impl();
            
            if (m_worker.joinable()) m_worker.join();
        }
    }

    void flush() {
        std::lock_guard<std::mutex> lock{m_mutex};
        flush_internal();
    }
    
    auto get_batch_size() const -> size_t {
        return Traits::BatchSize_;
    }

    auto get_pending_count() const -> size_t {
        std::lock_guard<std::mutex> lock{m_mutex};
        return m_strategy.batch.size();
    }

private:
    void worker_thread_func() {
        std::this_thread::sleep_for(std::chrono::milliseconds(Traits::FlushIntervalMs_ / 2));

        {
            std::lock_guard<std::mutex> lock{m_mutex};
            if (m_strategy.should_flush()) flush_internal();
        }

        {
            std::lock_guard<std::mutex> lock{m_mutex};
            flush_internal();
        }
    }

    void flush_internal() {
        m_strategy.flush(m_backend_logger);
    }

    std::atomic<bool> m_done;
    mutable std::mutex m_mutex;

    BatchStrategy<Traits::BatchSize_, Traits::FlushIntervalMs_> m_strategy;
    AsyncLogger<typename Traits::BackendPolicy_> m_backend_logger;
    std::thread m_worker;
};

template <typename Traits>
BatchAsyncLogger<Traits>::BatchAsyncLogger()
    : pImpl{std::make_unique<Impl>()} { }

template <typename Traits>
BatchAsyncLogger<Traits>::~BatchAsyncLogger() = default;

template <typename Traits>
void BatchAsyncLogger<Traits>::log_impl(std::string message) {
    pImpl->log(std::move(message));
}

template <typename Traits>
void BatchAsyncLogger<Traits>::stop_impl() {
    pImpl->stop();
}

template <typename Traits>
void BatchAsyncLogger<Traits>::flush() {
    pImpl->flush();
}

template <typename Traits>
auto BatchAsyncLogger<Traits>::get_batch_size() const -> size_t {
    return pImpl->get_batch_size();
}

template <typename Traits>
auto BatchAsyncLogger<Traits>::get_pending_count() const -> size_t {
    return pImpl->get_pending_count();
}





} // namespace v2
} // namespace labelimg::core::logger
#endif // ASYNC_LOGGER_H