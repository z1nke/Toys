#ifndef Y64_LIB_INSTRUCTION_HPP
#define Y64_LIB_INSTRUCTION_HPP

#include "buffer.hpp"
#include "register.hpp"

namespace y64 {

/// Y64 machine instruction and pseudo-instruction
/// <icode> <ifun> [regA] [regB] [value]
class Instruction {
public:
  enum ICode : std::uint8_t {
#define INST(NAME, ICODE, IFUN) icode_##NAME = ICODE,
#define INST_COND(NAME, ICODE)  icode_##NAME = ICODE,
  #include "insts.def"
  };

  enum IFun : std::uint8_t {
#define COND(NAME, IFUN)  ifun_##NAME = IFUN,
  #include "insts.def"
  };

  enum OpCode : std::uint8_t {
#define INST(NAME, ICODE, IFUN) NAME = ((ICODE << 4) | IFUN),
#define INST_COND(NAME, ICODE) \
    INST(NAME##le, ICODE, 0x1) \
    INST(NAME##l,  ICODE, 0x2) \
    INST(NAME##e,  ICODE, 0x3) \
    INST(NAME##ne, ICODE, 0x4) \
    INST(NAME##ge, ICODE, 0x5) \
    INST(NAME##g,  ICODE, 0x6)
  #include "insts.def"
  };


public:
  Instruction()
      : value(), addr(0), line(), icode(), ifun(), regA(), regB(),
        isPseduo(), isPendingAddress(), hasAddr(false) { }

  Instruction(const Instruction&) = default;
  Instruction& operator=(const Instruction&) = default;

  std::size_t length() const;

  void setOpCode(std::uint8_t opcode) {
    icode = opcode >> 4;
    ifun = opcode & 0xF;
  }

  std::uint8_t getOpCode() const {
    return (icode << 4) | ifun;
  }

  std::uint8_t getRegister() const {
    return regA.id() << 4 | regB.id();
  }

  void setAddress(std::uint64_t addr) {
    this->addr = addr;
    hasAddr = true;
  }

  void emit(InstBuffer& buf, std::size_t& len) const;

public:
  std::int64_t value;
  std::uint64_t addr;
  int line;
  std::uint8_t icode;
  std::uint8_t ifun;
  Register regA;
  Register regB;
  bool isPseduo;
  bool isPendingAddress;
  bool hasAddr;
};

} // class Instruction

#endif // !Y64_LIB_INSTRUCTION_HPP