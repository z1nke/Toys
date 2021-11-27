#include "util.hpp"

#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <fstream>
#include <iostream>

#include "y64exception.hpp"

#if defined __has_builtin
#if __has_builtin(__builtin_unreachable)
#define Y64_BUILTIN_UNREACHABLE __builtin_unreachable()
#endif // __has_builtin(__builtin_unreachable)
#endif // defined __has_builtin

#if !defined(Y64_BUILTIN_UNREACHABLE) && defined(_MSC_VER)
#define Y64_BUILTIN_UNREACHABLE __assume(false)
#endif // !defined(Y64_BUILTIN_UNREACHABLE) && defined(_MSC_VER)

#ifndef Y64_BUILTIN_UNREACHABLE
#define Y64_BUILTIN_UNREACHABLE
#endif // !Y64_BUILTIN_UNREACHABLE

[[noreturn]] void y64::details::y64_unreachable_internal(const char *msg,
                                                         const char *filename,
                                                         unsigned lineno) {
#ifndef NDEBUG // DEBUG
  if (msg) {
    std::cerr << msg << std::endl;
  }
  std::cerr << "Unreachable executed";
  if (filename) {
    std::cerr << " at " << filename << ":" << lineno;
  }
  std::cerr << "!\n";
#endif // !NDEBUG
  abort();
  Y64_BUILTIN_UNREACHABLE;
}

static const std::size_t kOutputBufferSize = 256;

void y64::parseError(const char *fmt, ...) {
  char buffer[kOutputBufferSize];
  std::va_list args;
  va_start(args, fmt);
  int len = std::vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);
  assert(len < static_cast<int>(sizeof(buffer)));
  buffer[len] = '\0';
  throw ParsingException{buffer};
}

const char *y64::magicNumber = "y64;";

bool y64::readSource(const std::string &filename, std::string &source) {
  std::ifstream input(filename, std::ios::binary);

  if (!input.is_open()) {
    return false;
  }

  source.assign(std::istreambuf_iterator<char>(input),
                std::istreambuf_iterator<char>());
  input.close();

  return true;
}
