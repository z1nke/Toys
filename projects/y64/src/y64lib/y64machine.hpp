#ifndef Y64_LIB_Y64_MACHINE_HPP
#define Y64_LIB_Y64_MACHINE_HPP

#include <array>
#include <vector>

#include "buffer.hpp"
#include "instruction.hpp"
#include "register.hpp"

namespace y64 {

class Machine {
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
  Machine()
      : mem(std::vector<std::uint8_t>(kMemorySize)), pc(0), zeroFlag(0),
        signedFlag(0), overflowFlag(0), stat(Stat::AOK), valA(0), valB(0),
        valC(0), valE(0), valM(0), valP(0), cnd(false), inst() {
#define REGISTER(NAME, STR, ID) NAME = Register::make(ID);
#include "registers.def"

    valueRegs.fill(0);
  }

public:
  // Load bytes buffer or file to memory
  bool load(const std::vector<std::uint8_t> &buffer);
  bool load(const std::string &filename);

  // Fetch, decode, excute, memory, write back, update PC
  // See https://w3.cs.jmu.edu/lam2mo/cs261_2018_08/files/y86-isa.pdf
  void fetch();
  void decode();
  void execute();
  void accessMemory();
  void writeBack();
  void updatePC();

  bool isOk() const {
    return stat == Stat::AOK;
  }

private:
  std::uint8_t readMemByte(std::uint64_t addr);
  std::int64_t readMemQuad(std::uint64_t addr);
  void writeMemQuad(std::uint64_t addr, std::int64_t val);
  void writeMemInst(std::uint64_t addr, const InstBuffer &buf);
  bool getCondition();
  void executeOpInst();

public:
  // For debugging
  void printAllRegs() const;
  void printGenRegs() const;
  void printMemory(std::uint64_t offset, std::uint64_t len) const;

private:
  void printLineMemoryByte(std::uint64_t offset, std::uint64_t n,
                           void (*print)(std::uint8_t)) const;

private:
  std::vector<std::uint8_t> mem;
  std::uint64_t pc;
  std::uint8_t zeroFlag;
  std::uint8_t signedFlag;
  std::uint8_t overflowFlag;
  Stat stat;

  // value registers, save temporary results
  std::int64_t valA;  // R[ra] in instruction
  std::int64_t valB;  // R[rb] in instruction or R[%rsp]
  std::int64_t valC;  // 8 bytes value in instruction
  std::int64_t valE;  // temporary result
  std::int64_t valM;  // 8 bytes memory value
  std::uint64_t valP; // R[PC]
  bool cnd;

  // For get icode:ifun and rA:rB
  Instruction inst;

// General registers
#define REGISTER(NAME, STR, ID) Register NAME;
#include "registers.def"

  std::array<std::int64_t, kNumGeneralRegs> valueRegs;
};

} // namespace y64

#endif // !Y64_LIB_Y64_MACHINE_HPP
