#ifndef Y64_LIB_BUFFER_HPP
#define Y64_LIB_BUFFER_HPP

#include <array>

#include "util.hpp"

namespace y64 {

class InstBuffer {
public:
  InstBuffer() : store(), len(0) { }

  std::size_t size() const {
    return len;
  }

  const std::array<std::uint8_t, kMaxInstLen>& data() {
    return store;
  }

  void append(std::uint8_t u8) {
    store[len++] = u8;
  }

  void append(std::int64_t i64) {
    Value64 v;
    v.i64 = i64;
    if (Endian::native == Endian::little) {
      for (int i = 0; i < 8; ++i) {
        append(v.arr[i]);
      }
    } else {
      for (int i = 7; i >= 0; --i) {
        append(v.arr[i]);
      }
    }
  }

  void append(std::uint64_t u64) {
    return append(static_cast<std::int64_t>(u64));
  }

  std::uint8_t retrieveByte() {
    return store[--len];
  }

  std::int64_t retrieveI64() {
    Value64 v;
    if (Endian::native == Endian::little) {
      for (int i = 7; i >= 0; --i) {
        v.arr[i] = retrieveByte();
      }
    } else {
      for (int i = 0; i < 8; ++i) {
        v.arr[i] = retrieveByte();
      }
    }

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

#endif //!Y64_LIB_BUFFER_HPP