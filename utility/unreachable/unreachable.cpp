#include "unreachable.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace toys {
namespace details {

#if defined __has_builtin
#if __has_builtin(__builtin_unreachable)
#define TOYS_BUILTIN_UNREACHABLE __builtin_unreachable()
#endif // __has_builtin(__builtin_unreachable)
#endif // defined __has_builtin

#if !defined(TOYS_BUILTIN_UNREACHABLE) && defined(_MSC_VER)
#define TOYS_BUILTIN_UNREACHABLE __assume(false)
#endif // !defined(BUILTIN_UNREACHABLE) && defined(_MSC_VER)

#ifndef TOYS_BUILTIN_UNREACHABLE
#define TOYS_BUILTIN_UNREACHABLE
#endif

[[noreturn]] void unreachableInternal(const char *msg, const char *filename,
                                      unsigned lineno) {
#ifndef NDEBUG // DEBUG
  if (msg)
    std::fprintf(stderr, "%s\n", msg);
  std::fputs("Unreachable executed", stderr);
  if (filename)
    std::fprintf(stderr, " at %s:%u", filename, lineno);
  std::fputs("!\n", stderr);
#endif
  std::abort();
  TOYS_BUILTIN_UNREACHABLE;
}

} // namespace details
} // namespace toys
