#ifndef Y64_LIB_BUFFER_HPP
#define Y64_LIB_BUFFER_HPP

#include <array>
#include <cassert>
#include <cstring>

#include "util.hpp"

namespace y64 {

class InstBuffer {
public:
  InstBuffer() : store(), len(0) {}

  std::size_t size() const { return len; }

  const std::array<std::uint8_t, kMaxInstLen> &data() const { return store; }

  std::array<std::uint8_t, kMaxInstLen> &data() { return store; }

  void append(std::uint8_t u8) {
    assert(len < kMaxInstLen);
    store[len++] = u8;
  }

  void append(const std::uint8_t *src, std::size_t n) {
    assert(n + len <= kMaxInstLen);
    std::memmove(store.data() + len, src, n);
    len += n;
  }

  void append(std::int64_t i64) {
    assert(len + sizeof(std::int64_t) <= kMaxInstLen);
    Value64 v;
    v.i64 = i64;
    std::memmove(store.data() + len, v.arr, sizeof(std::int64_t));
    len += sizeof(std::int64_t);
  }

  void append(std::uint64_t u64) {
    return append(static_cast<std::int64_t>(u64));
  }

  std::uint8_t retrieveByte() {
    assert(len >= sizeof(std::uint8_t));
    return store[--len];
  }

  std::int64_t retrieveI64() {
    assert(len >= sizeof(std::int64_t));
    Value64 v;
    v.i64 = 0;
    std::memmove(v.arr, store.data() + len - sizeof(std::int64_t),
                 sizeof(std::int64_t));
    len -= sizeof(std::int64_t);
    return v.i64;
  }

  std::uint64_t retrieveU64() {
    return static_cast<std::uint64_t>(retrieveI64());
  }

private:
  union Value64 {
    std::int64_t i64;
    std::uint64_t u64;
    std::uint8_t arr[8];
  };

private:
  std::array<std::uint8_t, kMaxInstLen> store;
  std::size_t len;
};

} // namespace y64

#endif //! Y64_LIB_BUFFER_HPP