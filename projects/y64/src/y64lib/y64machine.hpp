#ifndef Y64_LIB_Y64_MACHINE_HPP
#define Y64_LIB_Y64_MACHINE_HPP

#include <array>
#include <vector>

#include "buffer.hpp"
#include "instruction.hpp"
#include "register.hpp"

namespace y64 {

class Y64Machine {
public:
  static const std::uint64_t kMemorySize = 0x2000;
  static const std::size_t kNumGeneralRegs = 15;

  enum Stat : std::uint8_t {
    AOK,
    HLT, // execute `halt` instruction
    ADR, // invalid address
    INS, // invalid instruction
  };

public:
  Y64Machine() : mem(std::vector<std::uint8_t>(kMemorySize)),
                 pc(0), zeroFlag(false), signedFlag(false), overflowFlag(false),
                 stat(Stat::AOK), valA(0), valB(0), valC(0), valE(0), valM(0),
                 valP(0), inst() {
    #define REGISTER(NAME, STR, ID) NAME = Register::make(ID);
    #include "registers.def"

    valueRegs.fill(0);
  }

public:
  // Load bytes buffer or file to memory
  void load(const std::vector<std::uint8_t>& buffer);
  void load(const std::string& filename);

  // Fetch, decode, excute, memory, write back, update PC
  // See https://w3.cs.jmu.edu/lam2mo/cs261_2018_08/files/y86-isa.pdf
  void fetch();
  void decode();
  void execute();
  void accessMemory();
  void writeBack();
  void updatePC();

private:
  std::uint8_t getMemByte(std::uint64_t addr);
  std::int64_t getMemQuad(std::uint64_t addr);

public:
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

  // value registers, save temporary results
  std::int64_t valA;   // R[ra] in instruction
  std::int64_t valB;   // R[rb] in instruction or R[%rsp]
  std::int64_t valC;   // 8 bytes value in instruction
  std::int64_t valE;   // temporary result
  std::int64_t valM;   // 8 bytes memory value
  std::uint64_t valP;  // R[PC]

  // For get icode:ifun and rA:rB
  Instruction inst;

  // General registers
  #define REGISTER(NAME, STR, ID) Register NAME;
  #include "registers.def"

  std::array<std::int64_t, kNumGeneralRegs> valueRegs;
};

} // namespace y64

#endif // !Y64_LIB_Y64_MACHINE_HPP
