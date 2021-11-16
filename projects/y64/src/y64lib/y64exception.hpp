#ifndef Y64_LIB_Y64_EXCEPTION_HPP
#define Y64_LIB_Y64_EXCEPTION_HPP

#include <stdexcept>

#define DEFINE_EXCEPTION_CLASS(CLASS_NAME)                          \
class CLASS_NAME : public std::exception {                          \
public:                                                             \
  explicit CLASS_NAME(const char* msg) noexcept                     \
     : std::exception(), message(msg) { }                           \
  CLASS_NAME(const CLASS_NAME&) = default;                          \
  ~CLASS_NAME() noexcept override { }                               \
  CLASS_NAME& operator=(const CLASS_NAME& rhs) = default;           \
  const char* what() const noexcept override {                      \
    return message.c_str();                                         \
  }                                                                 \
private:                                                            \
  std::string message;                                              \
}

namespace y64 {
DEFINE_EXCEPTION_CLASS(ParsingException);
DEFINE_EXCEPTION_CLASS(RunningException);
} // namespace y64

#endif //!Y64_LIB_Y64_EXCEPTION_HPP