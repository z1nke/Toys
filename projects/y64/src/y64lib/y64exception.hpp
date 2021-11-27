#ifndef Y64_LIB_Y64_EXCEPTION_HPP
#define Y64_LIB_Y64_EXCEPTION_HPP

#include <stdexcept>

namespace y64 {
class ParsingException : public std::exception {
public:
  explicit ParsingException(const char *msg) noexcept
      : std::exception(), message(msg) {}
  ParsingException(const ParsingException &) = default;
  ~ParsingException() noexcept override {}
  ParsingException &operator=(const ParsingException &rhs) = default;
  const char *what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};

class RunningException : public std::exception {
public:
  explicit RunningException(std::uint8_t stat, std::uint64_t value) noexcept
      : std::exception(), stat(stat), value(value) {}
  RunningException(const RunningException &) = default;
  ~RunningException() noexcept override {}
  RunningException &operator=(const RunningException &rhs) = default;

  std::uint8_t getStat() const { return stat; }

  std::uint64_t getValue() const { return value; }

private:
  std::uint8_t stat;
  std::uint64_t value;
};
} // namespace y64

#endif //! Y64_LIB_Y64_EXCEPTION_HPP