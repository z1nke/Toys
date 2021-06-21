#ifndef Y64_LIB_REGISTER_HPP
#define Y64_LIB_REGISTER_HPP

#include <string>

namespace y64 {

class Register {
public:
  enum Reg : std::uint8_t {
#define REGISTER(NAME, STR, ID) NAME = ID,
#include "registers.def"
    none = 0xF,
    err
  };

  std::string name();
  std::uint8_t id() const {
    return static_cast<std::uint8_t>(reg);
  }

  Register() : reg(err) {}
  Register(const Register&) = default;
  Register& operator=(const Register&) = default;

  bool isErr() {
    return reg == err;
  }

  static Register make(int line, const std::string& name);
  static Register makeNone();

private:
  Register(Reg r) : reg(r) {}

  // name: register name, such as %rax, %rsp, etc.
  Register(const std::string& name);

private:
  Reg reg;
};

} // namespace y64

#endif // !Y64_LIB_Y64_LEXER_HPP