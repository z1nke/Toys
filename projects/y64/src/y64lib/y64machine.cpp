#include "y64machine.hpp"

#include <iostream>
#include <iomanip>

#include "buffer.hpp"
#include "y64exception.hpp"
#include "util.hpp"

namespace y64 {

void Y64Machine::printGenRegs() const {
  #define REGISTER(NAME, STR, ID) do {              \
    std::uint64_t value = valueRegs[ID];            \
    std::cout << #STR << ": " << value << '\n';     \
  } while (0);
  #include "registers.def"
}

const char* statToStr(Y64Machine::Stat stat) {
  switch (stat) {
  case Y64Machine::Stat::AOK:
    return "AOK";
  case Y64Machine::Stat::HLT:
    return "HLT";
  case Y64Machine::Stat::ADR:
    return "ADR";
  case Y64Machine::Stat::INS:
    return "INS";
  default:
    Y64_UNREACHABLE("Unknown machine state");
  }
}

void Y64Machine::fetch() {
  inst.setOpCode(getMemByte(pc));
  switch (inst.icode) {
  case Instruction::icode_halt:
  case Instruction::icode_nop:
  case Instruction::icode_ret:
    valP = pc + 1;
    break;
  case Instruction::icode_cmov:
  case Instruction::icode_addq:
  case Instruction::icode_pushq:
  case Instruction::icode_popq:
    inst.setRegister(getMemByte(pc + 1));
    valP = pc + 2;
    break;
  case Instruction::icode_j:
  case Instruction::icode_call:
    valC = getMemQuad(pc + 1);
    valP = pc + 9;
    break;
  case Instruction::icode_irmovq:
  case Instruction::icode_rmmovq:
  case Instruction::icode_mrmovq:
    inst.setRegister(getMemByte(pc + 1));
    valC = getMemQuad(pc + 2);
    valP = pc + 10;
    break;
  default:
    stat = Stat::INS;
    throw RunningException{ stat, inst.getOpCode() };
  }
}

void Y64Machine::decode() {
  switch (inst.icode) {
  case Instruction::icode_cmov:
    valA = valueRegs[inst.regA.id()];
    break;
  case Instruction::icode_mrmovq:
    valB = valueRegs[inst.regB.id()];
    break;
  case Instruction::icode_rmmovq:
    valA = valueRegs[inst.regA.id()];
    valB = valueRegs[inst.regB.id()];
    break;
  case Instruction::icode_call:
    valB = valueRegs[rsp.id()];
    break;
  case Instruction::icode_ret:
  case Instruction::icode_popq:
    valA = valueRegs[rsp.id()];
    valB = valueRegs[rsp.id()];
    break;
  case Instruction::icode_pushq:
    valA = valueRegs[inst.regA.id()];
    valB = valueRegs[rsp.id()];
    break;
  default:
    break;
  }
}

void Y64Machine::printAllRegs() const {
  std::cout << "pc:" << pc << '\t'
            << "ZF:" << zeroFlag << ' '
            << "SF:" << signedFlag << ' '
            << "OF:" << overflowFlag << '\t'
            << "Stat:" << statToStr(stat) << "\n";
  printGenRegs();
}

std::uint8_t Y64Machine::getMemByte(std::uint64_t addr) {
  if (addr >= mem.size()) {
    stat = Stat::ADR;
    throw RunningException{ stat, addr };
  }
  return mem[addr];
}

std::int64_t Y64Machine::getMemQuad(std::uint64_t addr) {
  if (mem.size() < 8 || addr > mem.size() - 8) {
    stat = Stat::ADR;
    throw RunningException{ stat, addr + 8 };
  }

  InstBuffer buf;
  buf.append(mem.data() + addr, 8);
  return buf.retrieveI64();
}


void Y64Machine::printLineMemoryByte(std::uint64_t offset, std::uint64_t n,
                                     void(*print)(std::uint8_t)) const {
  for (int i = 0; i < n && offset < kMemorySize; ++i) {
    print(mem[offset++]);
  }
}

void printHexMemoryByte(std::uint8_t val) {
  std::cout << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<std::uint32_t>(val) << ' ';
}

void printCharMemoryByte(std::uint8_t val) {
  if (std::isprint(val)) {
    std::cout << val;
  } else {
    std::cout << '.';
  }
}

void Y64Machine::printMemory(std::uint64_t offset, std::uint64_t len) const {
  // Print memory space with 16 bytes per line
  static const std::uint64_t bytesPerLine = 16;
  std::cout << std::right << std::setw(10) << "offset" << ": ";
  for (int i = 0; i < bytesPerLine; ++i) {
    std::cout << std::setiosflags(std::ios::uppercase) << std::hex
              << std::setw(2) << std::setfill('0') << i << ' ';
  }
  std::cout << '\n';
  for (std::uint64_t i = 0; offset < kMemorySize && i < len; ) {
    std::cout << "0x" << std::hex << std::setw(8)
              << std::setfill('0') << offset << ": ";

    int n = std::min(bytesPerLine, len - i);
    printLineMemoryByte(offset, n, printHexMemoryByte);
    std::cout << ' ';
    printLineMemoryByte(offset, n, printCharMemoryByte);
    std::cout << '\n';

    i += n;
    offset += n;
  }
}

} // namespace y64
