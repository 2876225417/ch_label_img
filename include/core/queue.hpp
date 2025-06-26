#ifndef QUEUE_H
#define QUEUE_H

#include <pch.h>

namespace labelimg::core::queue {
namespace v1 {
template <typename T>
class
#if defined(_DEPRECATED_INFO_) 
[[deprecated("The 'Queue' class template is deprecated. Please consider a CRTP base 'Queue' class template.")]]
#endif
Queue{
public:
    virtual ~Queue() = default;

    virtual void push(T value) { m_queue.push(std::move(value)); }
    virtual void pop() { m_queue.pop(); }
    virtual auto front() -> T& { return m_queue.front();  };
    virtual auto empty() -> bool { return m_queue.empty(); }
    virtual auto size() -> size_t { return m_queue.size(); }
protected:
    std::queue<T> m_queue;
};
} // namespace v1

inline namespace v2 {
template <typename Derived, typename T>
class Queue {
public:
    void push(T value) { 
        static_cast<Derived*>(this)->push_impl(std::move(value)); 
    }

    void pop() { 
        static_cast<Derived*>(this)->pop_impl(); 
    }

    auto front() -> T& {
        return static_cast<Derived*>(this)->front_impl(); 
    }

    [[nodiscard]] auto empty() 
    const -> bool { 
        return static_cast<Derived*>(this)->empty_impl(); 
    }

    [[nodiscard]] auto size()
    const -> size_t {
        return static_cast<Derived*>(this)->size_impl(); 
    }
protected:
    ~Queue() = default;
};
} // namespace v2

} // namespace labelimg::core::queue

#endif // QUEUE_H
