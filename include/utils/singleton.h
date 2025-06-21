#ifndef SINGLETON_H
#define SINGLETON_H

#include "utils/non-copyable.h"

template <typename Derived>
class Singleton: private NonCopyable {
public:
    static auto instance() -> Derived& {
        static Derived instance;
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

#define MAKE_SINGLETON(ClassName) \
    friend class Singleton<ClassName>; \
    private: \
        ClassName() = default; \
        ~ClassName() = default; 


#endif // SINGLETON_H