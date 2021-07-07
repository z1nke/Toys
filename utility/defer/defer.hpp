#pragma once

#include <utility>

namespace toys {
template <typename Callable>
class DeferOnScopeExit {
public:
    explicit DeferOnScopeExit(Callable&& func) 
        : mFunc(std::forward<Callable>(func)), mInvoke(true) { }

    DeferOnScopeExit(const DeferOnScopeExit&) = delete;
    DeferOnScopeExit(DeferOnScopeExit&& rhs) 
        : mFunc(std::move(rhs.mFunc)), mInvoke(rhs.mInvoke) {
        rhs.mInvoke = false;
    }
    DeferOnScopeExit& operator=(const DeferOnScopeExit&) = delete;

    ~DeferOnScopeExit() {
        if (mInvoke) {
            mFunc();
        }
    }
private:
    Callable mFunc;
    bool mInvoke;
};

struct DeferHelperTag {};

template <typename Callable>
constexpr DeferOnScopeExit<Callable>
operator|(DeferHelperTag, Callable&& func) {
    return DeferOnScopeExit<Callable>(std::forward<Callable>(func));
}

} // namespace toys

#ifdef __COUNTER__
#define DEFER_ID __COUNTER__
#else
#define DEFER_ID __LINE__
#endif

#define JOIN_STR(STR1, STR2) STR1 ## STR2
#define DEFER_IMPL(NAME, LINE) JOIN_STR(NAME, LINE)

#define DEFER                                            \
    auto DEFER_IMPL(DEFER_ON_SCOPE_EXIT_, DEFER_ID) =    \
        toys::DeferHelperTag{} | [&]()
