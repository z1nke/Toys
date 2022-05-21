#ifndef TOYS_UNREACHABLE_HPP
#define TOYS_UNREACHABLE_HPP

namespace toys {
namespace details {

[[noreturn]] void unreachableInternal(const char *msg, const char *filename,
                                      unsigned lineno);

} // namespace details
} // namespace toys

#define TOYS_UNREACHABLE(msg)                                                  \
  toys::details::unreachableInternal(msg, __FILE__, __LINE__)

#endif
