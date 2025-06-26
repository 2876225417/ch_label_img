#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <core/queue.hpp>

namespace labelimg::core::queue {
namespace v1 {
template <typename T>
class MessageQueue: public Queue<T>, private NonCopyable{
public:
    MessageQueue() = default;
    ~MessageQueue() override = default; 
public:
    void wait_and_pop(T&);
    auto try_pop(T&) -> bool;
    
    void push(T) override;
    auto empty() const -> bool;
    auto size()  const -> size_t;
private:    
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};

template <typename T>
void MessageQueue<T>::push(T value) {
    std::lock_guard<std::mutex> lock{m_mutex};

    Queue<T>::m_queue.push(std::move(value));

    m_cond.notify_one();
}

template <typename T>
void MessageQueue<T>::wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lock{m_mutex};

    m_cond.wait(lock, [this] { return !Queue<T>::m_queue.empty(); });

    value = std::move(Queue<T>::m_queue.front());
    Queue<T>::m_queue.pop();
}

template <typename T>
auto MessageQueue<T>::try_pop(T& value) -> bool {
    std::lock_guard<std::mutex> lock{m_mutex};
    if (Queue<T>::m_queue.empty()) return false;
    value = std::move(Queue<T>::m_queue.front());
    Queue<T>::m_queue.pop();
    return true;
}

template <typename T>
auto MessageQueue<T>::empty() const -> bool {
    std::lock_guard<std::mutex> lock{m_mutex};
    return Queue<T>::m_queue.empty();
}

template <typename T>
auto MessageQueue<T>::size() const -> size_t  {
    std::lock_guard<std::mutex> lock{m_mutex};
    return Queue<T>::m_queue.size();
}
} // namespace v1

inline namespace v2 {
struct MutexPolicy {};
struct CoroutinePolicy {};

template <typename T, typename ConcurrencyPolicy = MutexPolicy>
class MessageQueue;

template <typename T>
class MessageQueue<T, MutexPolicy>: public Queue<MessageQueue<T, MutexPolicy>, T>
                                  , private NonCopyable {
public:
    MessageQueue() = default;
    ~MessageQueue() = default;

    void wait_and_pop(T&);
    auto try_pop(T&) -> bool;
    auto wait_for_pop(T&, std::chrono::milliseconds timeout) -> bool;

    void push_impl(T);
    void pop_impl();
    auto front_impl() -> T&;
    [[nodiscard]] auto empty_impl() const -> bool;
    [[nodiscard]] auto size_impl()  const -> size_t;
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};

template <typename T>
void MessageQueue<T, MutexPolicy>::push_impl(T value) {
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_queue.push(std::move(value));
    }
    m_cond.notify_one();
}

template <typename T>
void MessageQueue<T, MutexPolicy>::pop_impl() {
    std::lock_guard<std::mutex> lock{m_mutex};
    if (!m_queue.empty()) m_queue.pop();
} 

template <typename T>
auto MessageQueue<T, MutexPolicy>::front_impl() -> T& {
    std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.front();
}

template <typename T>
auto MessageQueue<T, MutexPolicy>::empty_impl() const -> bool {
    std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.empty();
}

template <typename T>
auto MessageQueue<T, MutexPolicy>::size_impl() const -> size_t {
    std::lock_guard<std::mutex> lock{m_mutex};
    return m_queue.size();
}

template <typename T>
void MessageQueue<T>::wait_and_pop(T& value) {
    std::unique_lock<std::mutex> lock{m_mutex};
    m_cond.wait(lock, [this] { return !m_queue.empty(); });
    value = std::move(m_queue.front());
    m_queue.pop();
}

template <typename T>
auto MessageQueue<T>::try_pop(T& value) -> bool {
    std::lock_guard<std::mutex> lock{m_mutex};
    if (m_queue.empty()) return false;
    value = std::move(m_queue.front());
    m_queue.pop();
    return true;
}

template <typename T>
auto MessageQueue<T>::wait_for_pop(T& value, std::chrono::milliseconds timeout) -> bool {
    std::unique_lock<std::mutex> lock{m_mutex};
    if (!m_cond.wait_for(lock, timeout, [this] { return !m_queue.empty(); })) 
        return false;

    value = std::move(m_queue.front());
    m_queue.pop();
    return true;
}

template <typename T>
class MessageQueue<T, CoroutinePolicy>: public Queue<MessageQueue<T, CoroutinePolicy>, T>
                                      , private NonCopyable {
public:
    MessageQueue() = default;
    ~MessageQueue() = default;

    struct PopPolicy {};
    struct PushPolicy {};
    // template args

    struct PopAwaiter {
        MessageQueue* queue;
        std::optional<T> value;

        [[nodiscard]] auto 
        await_ready() const noexcept -> bool {
            return !queue->m_queue.empty();
        }

        void await_suspend(std::coroutine_handle<> handle) {
            queue->m_waiting_cosumers.push_back(handle);
        }

        auto await_resume() -> T {
            if (!value.has_value()) {
                value = std::move(queue->m_queue.front());
                queue->m_queue.pop();
            }
            return std::move(value.value());
        }
    };

    auto async_pop() -> PopAwaiter {
        return PopAwaiter{this, std::nullopt};
    }

    struct PushAwaiter {
        MessageQueue* queue;
        T value;
        
        [[nodiscard]] auto 
        await_ready() const noexcept -> bool {
            return true;
        }

        void await_suspend([[maybe_unused]]std::coroutine_handle<> handle) noexcept {}

        auto await_resume() -> T {
            queue->push_impl(std::move(value));
        }
    };

    auto async_push(T value) -> PushAwaiter {
        return PushAwaiter{this, std::move(value)};
    }

    void push_impl(T);
    void pop_impl();
    auto front_impl() -> T&;
    [[nodiscard]] auto empty_impl() const -> bool;
    [[nodiscard]] auto size_impl() const -> size_t;
    
    auto try_pop() -> std::optional<T>;
private:
    std::queue<T> m_queue;
    std::list<std::coroutine_handle<>> m_waiting_cosumers;

    void resume_one_waiter() {
        if (!m_waiting_cosumers.empty()) {
            auto handle = m_waiting_cosumers.front();
            m_waiting_cosumers.pop_front();
            handle.resume();
        }
    }
};

template <typename T>
void MessageQueue<T, CoroutinePolicy>::push_impl(T value) {
    m_queue.push(std::move(value));
    resume_one_waiter();
}

template <typename T>
void MessageQueue<T, CoroutinePolicy>::pop_impl() {
    if (!m_queue.empty()) m_queue.pop();
}

template <typename T>
auto MessageQueue<T, CoroutinePolicy>::front_impl() -> T& {
    return m_queue.front();
}

template <typename T>
auto MessageQueue<T, CoroutinePolicy>::empty_impl() const -> bool {
    return m_queue.empty();
}

template <typename T>
auto MessageQueue<T, CoroutinePolicy>::size_impl() const -> size_t {
    return m_queue.size();
}

template <typename T>
auto MessageQueue<T, CoroutinePolicy>::try_pop() -> std::optional<T> {
    if (m_queue.empty()) return std::nullopt;
    T value = std::move(m_queue.front());
    m_queue.pop();
    return value;
}

template <typename T = void>
class Task {
public:
    struct promise_type {
        std::variant<std::monostate, T, std::exception_ptr> result;
        
        auto get_return_object() -> Task {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto initial_suspend() noexcept -> std::suspend_never  { return {}; }
        auto final_suspend()   noexcept -> std::suspend_always { return {}; }

        void return_value(T value) {
            result = std::move(value);
        }

        void unhanled_exception() {
            result = std::current_exception();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    explicit Task(handle_type h): m_handle{h} {}
    ~Task() {
        if (m_handle) m_handle.destroy();
    }

    Task(Task&& other) noexcept
        : m_handle{std::exchange(other.m_handle, {})} {}

    auto operator=(Task&& other) noexcept -> Task& {
        if (this != &other) {
            if (m_handle) m_handle.destroy();
            m_handle = std::exchange(other.m_handle, {});
        }
        return *this;
    }

    Task(const Task&) = delete;
    auto operator=(const Task&) -> Task& = delete;

    auto get() -> T {
        if (!m_handle || !m_handle.done()) {
            throw std::runtime_error("Task not completed");
        }

        auto& result = m_handle.promise().result;
        if (std::holds_alternative<std::exception_ptr>(result)) {
            std::rethrow_exception(std::get<std::exception_ptr>(result));
        }
        return std::get<T>(result);
    }

    [[nodiscard]] auto 
    is_ready() const -> bool {
        return m_handle && m_handle.done();
    }
private:
    handle_type m_handle;
};
template <>
class Task<void> {
public:
    struct promise_type {
        std::exception_ptr exception;
        
        auto get_return_object() -> Task {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        auto initial_suspend() noexcept -> std::suspend_never { return {}; }
        auto final_suspend() noexcept -> std::suspend_always { return {}; }

        void return_void() { }
        void unhandled_exception() {
            exception = std::current_exception();
        }
    };

    using handle_type = std::coroutine_handle<promise_type>;

    explicit Task(handle_type h) : m_handle{h} {}
    ~Task() {
        if (m_handle) m_handle.destroy();
    }

    Task(Task&& other) noexcept: m_handle{std::exchange(other.m_handle, {})} {}
    auto operator=(Task&& other) noexcept -> Task& {
        if (this != &other) {
            if (m_handle) m_handle.destroy();
            m_handle = std::exchange(other.m_handle ,{});
        }
        return *this;
    }

    Task(const Task&) = default;
    auto operator=(const Task&) -> Task& = delete;

    void get() {
        if (!m_handle || m_handle.done()) {
            throw std::runtime_error("Task not completed");
        }

        if (m_handle.promise().exception) {
            std::rethrow_exception(m_handle.promise().exception);
        }
    }

    [[nodiscard]] auto
    is_ready() -> bool {
        return m_handle && m_handle.done();
    } 

private:
    handle_type m_handle;
};

} // namespace v2

} // namespace labelimg::core::queue
#endif // MESSAGE_QUEUE_H