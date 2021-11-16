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
    #include "insts.def"
  };

  enum IFun : std::uint8_t {
    #define INST(NAME, ICODE, IFUN) ifun_##NAME = IFUN,
    #define COND(NAME, IFUN) ifun_##NAME = IFUN,
    #include "insts.def"
  };

  enum OpCode : std::uint8_t {
    #define INST(NAME, ICODE, IFUN) NAME = ((ICODE << 4) | IFUN),
    #include "insts.def"
  };


public:
  Instruction()
      : value(), addr(0), line(), icode(), ifun(), regA(), regB(),
        isPseduo(), isPendingAddress(), hasAddr(false) { }

  Instruction(const Instruction&) = default;
  Instruction& operator=(const Instruction&) = default;

  std::size_t length() const;
  static std::size_t length(std::uint8_t opcode);

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

  void setRegister(std::uint8_t reg) {
    regA = Register::make(reg >> 4);
    regB = Register::make(reg & 0xF);
  }

  void setAddress(std::uint64_t addr) {
    this->addr = addr;
    hasAddr = true;
  }

  void emit(InstBuffer& buf, std::size_t& len) const;

public:
  // value is:
  // (1) a pending dummy address of instruction or
  // (2) a value of pseduo instruction or
  // (3) an address of memory
  // (4) an immediate number
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
}; // class Instruction

} // namespace y64

#endif // !Y64_LIB_INSTRUCTION_HPP
