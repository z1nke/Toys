#ifndef Y64_LIB_REGISTER_HPP
#define Y64_LIB_REGISTER_HPP

#include <string>

namespace y64 {

class Register {
public:
  enum RID : std::uint8_t {
#define REGISTER(NAME, STR, ID) NAME = ID,
#include "registers.def"
    none = 0xF,
    err
  };

  std::string name() const;
  std::uint8_t id() const { return static_cast<std::uint8_t>(rid); }

  Register() : rid(err) {}
  Register(const Register &) = default;
  Register &operator=(const Register &) = default;

  bool isErr() const { return rid >= err; }

  // make register from y64 assembly code
  static Register make(int line, const std::string &name);
  static Register makeNone();

  // make register for y64 machine
  static Register make(std::uint8_t rid) {
    return Register{static_cast<RID>(rid)};
  }

private:
  // str: register str, such as %rax, %rsp, etc.
  explicit Register(const std::string &str);
  explicit Register(RID r) : rid(r) {}

private:
  RID rid;
};

} // namespace y64

#endif // !Y64_LIB_Y64_LEXER_HPP