#ifndef Y64_LIB_Y64_MACHINE_HPP
#define Y64_LIB_Y64_MACHINE_HPP

#include <array>
#include <vector>

#include "instruction.hpp"
#include "register.hpp"

namespace y64 {

class Y64Machine {
public:
  static const std::uint64_t kMemorySize = 0x2000;
  static const std::size_t kNumGeneralRegs = 15;

  enum Stat {
    AOK,
    HLT, // execute `halt` instruction
    ADR, // invalid address
    INS, // invalid instruction
  };

public:
  Y64Machine() : mem(std::vector<std::uint8_t>(kMemorySize)),
                 pc(0),
                 zeroFlag(false),
                 signedFlag(false),
                 overflowFlag(false),
                 stat(Stat::AOK) {
    #define REGISTER(NAME, STR, ID) NAME = Register::make(ID);
    #include "registers.def"

    valueRegs.fill(0);
  }

public:
  // Return 0 on success. On error, 1 is returned.
  int executeInstructions(const std::vector<Instruction>& insts);
  int executeInstruction(const Instruction& inst);

  // For debugging
  void printAllRegs() const;
  void printGenRegs() const;
  void printMemory(std::uint64_t offset, std::uint64_t len) const;

private:
  void printLineMemoryByte(std::uint64_t offset, std::uint64_t n,
                           void(*print)(std::uint8_t)) const;

private:
  std::vector<std::uint8_t> mem;
  std::uint64_t pc;
  bool zeroFlag;
  bool signedFlag;
  bool overflowFlag;
  Stat stat;

  // General registers
  #define REGISTER(NAME, STR, ID) Register NAME;
  #include "registers.def"

  std::array<std::uint64_t, kNumGeneralRegs> valueRegs;
};

} // namespace y64

#endif // !Y64_LIB_Y64_MACHINE_HPP