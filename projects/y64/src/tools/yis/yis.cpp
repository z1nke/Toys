// Y64 instruction simulator executable entry

#include <iomanip>
#include <iostream>

#include "../../y64lib/buffer.hpp"
#include "../../y64lib/instruction.hpp"
#include "../../y64lib/y64exception.hpp"
#include "../../y64lib/y64machine.hpp"
#include "../../y64lib/y64parser.hpp"

using namespace y64;

void usageHelp() {
  std::cerr << "yis - y86-64 simulator\n"
            << "yis [-yo|-ys] filename.[yo|ys]\n"
            << "Example: yis -yo foo.yo or yis -ys foo.ys\n";
}

int main(int argc, char **argv) {
  if (argc == 2) {
    std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help") {
      usageHelp();
      return 0;
    }
    usageHelp();
    return 1;
  }

  if (argc != 3) {
    usageHelp();
    return 1;
  }

  std::string opt = argv[1];
  if (opt != "-ys" && opt != "-yo") {
    usageHelp();
    return 1;
  }

  const char *filename = argv[2];
  fs::path sourcePath{filename};

  if (!fs::exists(sourcePath)) {
    std::cerr << "No such file: '" << filename << "'\n";
    return 2;
  }

  Machine cpu;
  if (opt == "-ys") {
    std::string source;
    if (!readSource(filename, source)) {
      std::cerr << "Read source file '" << filename << "' failed\n";
      return 2;
    }

    AsmParser parser{source};
    try {
      parser.parseStatements();
    } catch (ParsingException &e) {
      std::cerr << e.what() << "\n";
      return 2;
    }
    cpu.load(parser.getOutputBuffer());
  } else if (opt == "-yo") {
    cpu.load(filename);
  }

  cpu.printAllRegs();

  std::cout << "start execute!!!\n"
            << "An instruction will be executed, press any key to continue "
               "execution\n";

  char anykey = '\0';
  try {
    // cpu executes instructions until halt
    while (cpu.isOk()) {
      std::cin.get(anykey);
      cpu.fetch();
      cpu.decode();
      cpu.execute();
      cpu.accessMemory();
      cpu.writeBack();
      cpu.updatePC();
      cpu.printAllRegs();
    }

    std::cout << "mission completed!!!\n";
    return 0;
  } catch (RunningException &e) {
    std::uint8_t stat = e.getStat();
    if (stat == Machine::Stat::ADR) {
      std::cerr << "error: Invalid address: 0x" << std::hex
                << std::setiosflags(std::ios::uppercase) << e.getValue()
                << "\n";
      return 3;
    }

    if (stat == Machine::Stat::INS) {
      std::cerr << "error: Invalid instruction opcode: " << std::hex
                << std::setiosflags(std::ios::uppercase)
                << static_cast<std::uint8_t>(e.getValue()) << "\n";
      return 3;
    }
  }

  return 0;
}