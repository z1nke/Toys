#include "register.hpp"

#include <cassert>
#include <unordered_map>

#include "util.hpp"

namespace y64 {

std::string Register::name() {
  switch (reg) {
#define REGISTER(NAME, STR, ID) case ID: return #STR;
#include "registers.def"
  default:
    Y64_UNREACHABLE("Unknown register");
  }
}

static const std::unordered_map<std::string, Register::Reg> registersTable {
#define REGISTER(NAME, STR, ID) { std::string(#STR), Register::Reg(ID) },
#include "registers.def"
};

Register Register::make(int line, const std::string& name) {
  Register r{ name };
  if (r.isErr()) {
    parseError("%d: Unknown register: %s", line, name.c_str());
  }

  return r;
}

Register Register::makeNone() {
  return Register{ none };
}

Register::Register(const std::string& name) {
  auto iter = registersTable.find(name);
  if (iter == registersTable.end()) {
    reg = err;
    return;
  }

  reg = iter->second;
}

} // namespace y64