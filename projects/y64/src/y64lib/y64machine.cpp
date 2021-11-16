#include "y64machine.hpp"

#include <iostream>
#include <iomanip>

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

void Y64Machine::printAllRegs() const {
  std::cout << "pc:" << pc << '\t'
            << "ZF:" << zeroFlag << ' '
            << "SF:" << signedFlag << ' '
            << "OF:" << overflowFlag << '\t'
            << "Stat:" << statToStr(stat) << "\n";
  printGenRegs();
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
