#ifndef ASYNC_LOGGER_H
#define ASYNC_LOGGER_H

#include "utils/singleton.h"
#include <atomic>
#include <chrono>
#include <core/message_queue.hpp>
#include <csignal>
#include <memory>
#include <ostream>
#include <thread>
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
                                         , private Singleton<AsyncLogger<queue::CoroutinePolicy>> {
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

} // namespace v2
} // namespace labelimg::core::logger
#endif // ASYNC_LOGGER_H