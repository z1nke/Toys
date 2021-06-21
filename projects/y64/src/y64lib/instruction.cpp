#include "instruction.hpp"

#include "util.hpp"

namespace y64 {

std::size_t Instruction::length() const {
  switch (icode) {
  case icode_halt:
  case icode_nop:
  case icode_ret:
    return 1;
  case icode_rrmovq:
  case icode_addq: // OP
  case icode_pushq:
  case icode_popq:
    return 2;
  case icode_jmp:
  case icode_call:
    return 9;
  case icode_irmovq:
  case icode_rmmovq:
  case icode_mrmovq:
    return 10;
  default:
    Y64_UNREACHABLE("Unknown instruction");
  }
}

void Instruction::emit(InstBuffer& buf, std::size_t& len) const {
  buf.append(getOpCode());
  switch (icode) {
  case icode_halt:
  case icode_nop:
  case icode_ret:
    len = 1;
    return;
  case icode_rrmovq:
  case icode_addq: // OP
  case icode_pushq:
  case icode_popq:
    buf.append(getRegister());
    len = 2;
    return;
  case icode_jmp:
  case icode_call:
    buf.append(value);
    len = 9;
    return;
  case icode_irmovq:
  case icode_rmmovq:
  case icode_mrmovq:
    buf.append(getRegister());
    buf.append(value);
    len = 10;
    return;
  default:
    Y64_UNREACHABLE("Unknown instruction");
  }
}

} // namespace y64