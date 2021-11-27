#ifndef Y64_LIB_UTIL_HPP
#define Y64_LIB_UTIL_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#ifndef _WIN32
#include <endian.h>
#endif

namespace y64 {

constexpr std::size_t kMaxInstLen = 10; // C++17

[[noreturn]] void parseError(const char *fmt, ...);

enum class Endian {
#ifdef _WIN32
  little = 0,
  big = 1,
  native = little
#else
  little = __ORDER_LITTLE_ENDIAN__,
  big = __ORDER_BIG_ENDIAN__,
  native = __BYTE_ORDER__
#endif
};

extern const char *magicNumber;

bool readSource(const std::string &filename, std::string &source);

namespace details {
[[noreturn]] void y64_unreachable_internal(const char *msg,
                                           const char *filename,
                                           unsigned lineno);

template <typename Callable> class DeferOnScopeExit {
public:
  explicit DeferOnScopeExit(Callable &&f)
      : func(std::forward<Callable>(f)), invoke(true) {}

  DeferOnScopeExit(const DeferOnScopeExit &) = delete;
  DeferOnScopeExit(DeferOnScopeExit &&rhs)
      : func(std::move(rhs.func)), invoke(rhs.invoke) {
    rhs.invoke = false;
  }
  DeferOnScopeExit &operator=(const DeferOnScopeExit &) = delete;

  ~DeferOnScopeExit() {
    if (invoke)
      func();
  }

private:
  Callable func;
  bool invoke;
};

struct DeferHelperTag {};

template <typename Callable>
constexpr DeferOnScopeExit<Callable> operator|(DeferHelperTag,
                                               Callable &&func) {
  return DeferOnScopeExit<Callable>(std::forward<Callable>(func));
}

} // namespace details

} // namespace y64

#define Y64_UNREACHABLE(msg)                                                   \
  y64::details::y64_unreachable_internal(msg, __FILE__, __LINE__)

#if __cplusplus >= 201703L
#define Y64_FALLTHROUGH [[fallthrough]]
#elif defined(__clang__) && defined(__has_cpp_attribute)
#if __has_cpp_attribute(clang::fallthrough)
#define Y64_FALLTHROUGH [[clang::fallthrough]]
#endif
#elif defined(__GNUC__) && __GNUC__ > 6
#define Y64_FALLTHROUGH [[gnu::fallthrough]]
#else
#define Y64_FALLTHROUGH
#endif

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "Missing the <filesystem> header."
#endif

#ifdef __COUNTER__
#define Y64_DEFER_ID __COUNTER__
#else
#define Y64_DEFER_ID __LINE__
#endif

#define JOIN_STR(STR1, STR2) STR1##STR2
#define DEFER_IMPL(NAME, LINE) JOIN_STR(NAME, LINE)

#define DEFER                                                                  \
  auto DEFER_IMPL(DEFER_ON_SCOPE_EXIT_, Y64_DEFER_ID) =          \
      y64::details::DeferHelperTag{} | [&]()

#endif // !Y64_LIB_UTIL_HPP