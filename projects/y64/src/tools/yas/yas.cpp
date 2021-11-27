// yas -- y86-64 assembler

#include <fstream>
#include <iostream>

#include "../../y64lib/y64exception.hpp"
#include "../../y64lib/y64parser.hpp"

using namespace y64;
using namespace std;

namespace {

void usageHelp() { std::cerr << "Usage ./yas filename.ys\n"; }

} // namespace

int main(int argc, char **argv) {
  if (argc != 2) {
    usageHelp();
    return 1;
  }

  std::string arg = argv[1];
  if (arg == "-h" || arg == "--help") {
    usageHelp();
    return 0;
  }

  fs::path sourcePath{arg};

  if (!fs::exists(sourcePath)) {
    std::cerr << "No such file: '" << arg << "'\n";
    return 2;
  }

  std::string source;
  if (!readSource(arg, source)) {
    std::cerr << "Read source file '" << arg << "' failed\n";
    return 2;
  }

  fs::path outPath = sourcePath.replace_extension(".yo");
  AsmParser parser{source};

  try {
    parser.parseStatements();
  } catch (ParsingException &e) {
    cerr << e.what() << endl;
    return 1;
  }

  std::ofstream fout{outPath.c_str(), std::ios::binary};
  parser.emit(fout);

  return 0;
}