#ifndef Y64_LIB_Y64_EXCEPTION_HPP
#define Y64_LIB_Y64_EXCEPTION_HPP

#include <stdexcept>

namespace y64 {

class ParseException : public std::exception {
public:
  explicit ParseException(const char* msg) noexcept
     : std::exception(), message(msg) { }

  ParseException(const ParseException&) noexcept = default;
  ~ParseException() noexcept override { }
  ParseException& operator=(const ParseException& rhs) noexcept = default;

  const char* what() const noexcept override {
    return message.c_str();
  }

private:
  std::string message;
};

} // namespace y64

#endif //!Y64_LIB_Y64_EXCEPTION_HPP