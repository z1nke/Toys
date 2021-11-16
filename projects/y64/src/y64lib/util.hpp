#ifndef Y64_LIB_UTIL_HPP
#define Y64_LIB_UTIL_HPP

#include <cstdint>
#include <cstddef>

#ifndef _WIN32
#include <endian.h>
#endif

namespace y64 {

constexpr std::size_t kMaxInstLen = 10; // C++17

[[noreturn]]
void parseError(const char* fmt, ...);

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

namespace details {
[[noreturn]] void y64_unreachable_internal(const char* msg,
                                           const char* filename,
                                           unsigned lineno);
} // namespace details

extern const char* magicNumber;
} // namespace y64

#define Y64_UNREACHABLE(msg) \
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



#endif // !Y64_LIB_UTIL_HPP