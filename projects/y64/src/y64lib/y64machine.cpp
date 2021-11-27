#include "y64machine.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>

#include "buffer.hpp"
#include "util.hpp"
#include "y64exception.hpp"

namespace y64 {

static const char *statToStr(Machine::Stat stat) {
  switch (stat) {
  case Machine::Stat::AOK:
    return "AOK";
  case Machine::Stat::HLT:
    return "HLT";
  case Machine::Stat::ADR:
    return "ADR";
  case Machine::Stat::INS:
    return "INS";
  default:
    Y64_UNREACHABLE("Unknown machine state");
  }
}

static std::uint64_t convertU64(std::uint8_t opcode) {
  return static_cast<std::uint64_t>(opcode) & 0xFF;
}

// DEBUG
#define CHECK_BYTE(EXPECT, ACTUAL)                                             \
  if (static_cast<std::uint8_t>(EXPECT) != ACTUAL) {                           \
    std::cerr << __LINE__ << " error: Wrong file format";                      \
    return false;                                                              \
  }

// buffer:
// 0xaddr: inst1\n0xaddr: inst2\n...
bool Machine::load(const std::vector<std::uint8_t> &buffer) {
  CHECK_BYTE('\n', buffer.back());
  std::size_t idx = 0;
  while (idx < buffer.size()) {
    if (idx + 14 > buffer.size()) {
      std::cerr << "error: Wrong file format";
      return false;
    }
    CHECK_BYTE('0', buffer[idx++]);
    CHECK_BYTE('x', buffer[idx++]);
    InstBuffer addrBuf;
    addrBuf.append(buffer.data() + idx, 8);
    idx += 8;
    std::uint64_t addr = addrBuf.retrieveU64();
    CHECK_BYTE(':', buffer[idx++]);
    CHECK_BYTE(' ', buffer[idx++]);

    std::uint8_t opcode = buffer[idx];
    if (opcode == Instruction::dot_quad)
      ++idx;
    std::size_t lenInst = Instruction::length(opcode);
    InstBuffer instBuf;
    instBuf.append(buffer.data() + idx, lenInst);
    idx += lenInst;
    writeMemInst(addr, instBuf);
    CHECK_BYTE('\n', buffer[idx++]);
  }

  return true;
}

bool Machine::load(const std::string &filename) {
  std::ifstream fin{filename, std::ios_base::binary};
  if (!fin.is_open()) {
    std::cerr << "error: File '" << filename << "' open failed\n";
    return false;
  }
  DEFER { fin.close(); };

  fin.unsetf(std::ios::skipws);
  fin.seekg(0, std::ios::end);
  std::vector<std::uint8_t> buffer;
  std::size_t fileSize = fin.tellg();
  if (fileSize < 4) {
    std::cerr << "error: File '" << filename << "' is too small\n";
    return false;
  }
  fin.seekg(0, std::ios::beg);
  buffer.reserve(fileSize); // file size

  std::string magicHeader(4, '\0');
  fin.read(magicHeader.data(), 4);
  if (magicHeader != magicNumber) {
    std::cerr << "error: Wrong file format\n";
    return false;
  }

  buffer.insert(buffer.begin(), std::istream_iterator<std::uint8_t>(fin),
                std::istream_iterator<std::uint8_t>());
  return load(buffer);
}

void Machine::fetch() {
  inst.setOpCode(readMemByte(pc));
  switch (inst.icode) {
  case Instruction::icode_halt:
  case Instruction::icode_nop:
  case Instruction::icode_ret:
    valP = pc + 1;
    break;
  case Instruction::icode_cmov:
  case Instruction::icode_opq:
  case Instruction::icode_pushq:
  case Instruction::icode_popq:
    inst.setRegister(readMemByte(pc + 1));
    valP = pc + 2;
    break;
  case Instruction::icode_jmp:
  case Instruction::icode_call:
    valC = readMemQuad(pc + 1);
    valP = pc + 9;
    break;
  case Instruction::icode_irmovq:
  case Instruction::icode_rmmovq:
  case Instruction::icode_mrmovq:
    inst.setRegister(readMemByte(pc + 1));
    valC = readMemQuad(pc + 2);
    valP = pc + 10;
    break;
  default:
    stat = Stat::INS;
    throw RunningException{stat, convertU64(inst.getOpCode())};
  }
}

void Machine::decode() {
  switch (inst.icode) {
  case Instruction::icode_cmov:
    valA = valueRegs[inst.regA.id()];
    break;
  case Instruction::icode_mrmovq:
    valB = valueRegs[inst.regB.id()];
    break;
  case Instruction::icode_rmmovq:
    Y64_FALLTHROUGH;
  case Instruction::icode_opq:
    valA = valueRegs[inst.regA.id()];
    valB = valueRegs[inst.regB.id()];
    break;
  case Instruction::icode_call:
    valB = valueRegs[rsp.id()];
    break;
  case Instruction::icode_ret:
    Y64_FALLTHROUGH;
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

bool Machine::getCondition() {
  switch (inst.ifun) {
  case Instruction::ifun_jmp:
    return 1;
  case Instruction::ifun_le:
    return (signedFlag ^ overflowFlag) | zeroFlag;
  case Instruction::ifun_l:
    return signedFlag ^ overflowFlag;
  case Instruction::ifun_e:
    return zeroFlag == 1;
  case Instruction::ifun_ne:
    return zeroFlag == 0;
  case Instruction::ifun_ge:
    return signedFlag ^ overflowFlag ^ 1;
  case Instruction::ifun_g:
    return (signedFlag ^ overflowFlag ^ 1) & (zeroFlag ^ 1);
  default:
    stat = Stat::INS;
    throw RunningException{stat, convertU64(inst.getOpCode())};
    break;
  }
}

void Machine::executeOpInst() {
  switch (inst.ifun) {
  case Instruction::ifun_addq:
    valE = valB + valA;
    zeroFlag = valE == 0;
    signedFlag = valE < 0;
    overflowFlag = ((valA < 0) == (valB < 0)) && ((valE < 0) != (valA < 0));
    break;
  case Instruction::ifun_subq:
    valE = valB - valA;
    zeroFlag = valE == 0;
    signedFlag = valE < 0;
    overflowFlag = ((valA > 0) == (valB < 0)) && ((valE < 0) != (valB < 0));
    break;
  case Instruction::ifun_andq:
    valE = valB & valA;
    break;
  case Instruction::ifun_xorq:
    valE = valB ^ valA;
    break;
  default:
    stat = Stat::INS;
    throw RunningException{stat, convertU64(inst.getOpCode())};
  }
}

void Machine::execute() {
  switch (inst.icode) {
  case Instruction::icode_halt:
    stat = Stat::HLT;
    break;
  case Instruction::icode_cmov:
    valE = valA;
    cnd = getCondition();
    break;
  case Instruction::icode_irmovq:
    valE = valC;
    break;
  case Instruction::icode_rmmovq:
    Y64_FALLTHROUGH;
  case Instruction::icode_mrmovq:
    valE = valB + valC;
    break;
  case Instruction::icode_opq:
    executeOpInst();
    break;
  case Instruction::icode_jmp:
    cnd = getCondition();
    break;
  case Instruction::icode_call:
    Y64_FALLTHROUGH;
  case Instruction::icode_pushq:
    valE = valB - 8;
    break;
  case Instruction::icode_ret:
    Y64_FALLTHROUGH;
  case Instruction::icode_popq:
    valE = valB + 8;
    break;
  default:
    break;
  }
}

void Machine::accessMemory() {
  switch (inst.icode) {
  case Instruction::icode_rmmovq:
    writeMemQuad(valE, valA);
    break;
  case Instruction::icode_mrmovq:
    valM = readMemQuad(valE);
    break;
  case Instruction::icode_call:
    writeMemQuad(valE, valP);
    break;
  case Instruction::icode_ret:
    valM = readMemQuad(valA);
    break;
  case Instruction::icode_pushq:
    writeMemQuad(valE, valA);
    break;
  case Instruction::icode_popq:
    valM = readMemQuad(valA);
    break;
  default:
    break;
  }
}

void Machine::writeBack() {
  switch (inst.icode) {
  case Instruction::icode_cmov:
    if (cnd)
      valueRegs[inst.regB.id()] = valE;
    break;
  case Instruction::icode_irmovq:
    valueRegs[inst.regB.id()] = valE;
    break;
  case Instruction::icode_mrmovq:
    valueRegs[inst.regA.id()] = valM;
    break;
  case Instruction::icode_opq:
    valueRegs[inst.regB.id()] = valE;
    break;
  case Instruction::icode_call:
    Y64_FALLTHROUGH;
  case Instruction::icode_ret:
    Y64_FALLTHROUGH;
  case Instruction::icode_pushq:
    valueRegs[rsp.id()] = valE;
    break;
  case Instruction::icode_popq:
    valueRegs[rsp.id()] = valE;
    valueRegs[inst.regA.id()] = valM;
    break;
  default:
    break;
  }
}

void Machine::updatePC() {
  switch (inst.icode) {
  case Instruction::icode_halt:
    pc = 0;
    break;
  case Instruction::icode_j:
    pc = cnd ? valC : valP;
    break;
  case Instruction::icode_call:
    pc = valC;
    break;
  case Instruction::icode_ret:
    pc = valM;
    break;
  default:
    pc = valP;
    break;
  }
}

std::uint8_t Machine::readMemByte(std::uint64_t addr) {
  if (addr >= mem.size()) {
    stat = Stat::ADR;
    throw RunningException{stat, addr};
  }
  return mem[addr];
}

std::int64_t Machine::readMemQuad(std::uint64_t addr) {
  if (addr + sizeof(std::uint64_t) > mem.size()) {
    stat = Stat::ADR;
    throw RunningException{stat, addr + 8};
  }

  InstBuffer buf;
  buf.append(mem.data() + addr, 8);
  return buf.retrieveI64();
}

void Machine::writeMemQuad(std::uint64_t addr, std::int64_t val) {
  if (addr + sizeof(val) > mem.size()) {
    stat = Stat::ADR;
    throw RunningException{stat, addr + 8};
  }

  InstBuffer buf;
  buf.append(val);
  std::memmove(mem.data() + addr, buf.data().data(), buf.size());
}

void Machine::writeMemInst(std::uint64_t addr, const InstBuffer &buf) {
  if (addr + buf.size() > mem.size()) {
    stat = Stat::ADR;
    throw RunningException{stat, addr + 8};
  }

  std::memmove(mem.data() + addr, buf.data().data(), buf.size());
}

void Machine::printGenRegs() const {
#define REGISTER(NAME, STR, ID)                                                \
  do {                                                                         \
    std::uint64_t value = valueRegs[ID];                                       \
    std::cout << std::setw(4) << #STR << ": " << std::left << std::setw(15)    \
              << value << ((ID + 1) % 4 == 0 ? "\n" : " ");                    \
  } while (0);
#include "registers.def"
  std::cout << "\n";
}

void Machine::printAllRegs() const {
  std::cout << "PC:" << pc << '\t' << "ZF:" << convertU64(zeroFlag) << ' '
            << "SF:" << convertU64(signedFlag) << ' '
            << "OF:" << convertU64(overflowFlag) << '\t'
            << "Stat:" << statToStr(stat) << "\n";
  printGenRegs();
}

void Machine::printLineMemoryByte(std::uint64_t offset, std::uint64_t n,
                                  void (*print)(std::uint8_t)) const {
  for (std::uint64_t i = 0; i < n && offset < kMemorySize; ++i) {
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

void Machine::printMemory(std::uint64_t offset, std::uint64_t len) const {
  // Print memory space with 16 bytes per line
  static const std::uint64_t bytesPerLine = 16;
  std::cout << std::right << std::setw(10) << std::setfill(' ') << "offset"
            << ": ";
  for (std::uint64_t i = 0; i < bytesPerLine; ++i) {
    std::cout << std::setiosflags(std::ios::uppercase) << std::hex
              << std::setw(2) << std::setfill('0') << i << ' ';
  }
  std::cout << '\n';
  for (std::uint64_t i = 0; offset < kMemorySize && i < len;) {
    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << offset
              << ": ";

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
