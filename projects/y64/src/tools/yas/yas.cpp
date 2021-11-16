// yas -- y86-64 assembler

#include <iostream>
#include <fstream>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#error "Missing the <filesystem> header."
#endif

#include "../../y64lib/y64parser.hpp"
#include "../../y64lib/y64exception.hpp"

using namespace y64;
using namespace std;

namespace {

void usageHelp() {
  std::cerr << "Usage ./yas filename.ys\n";
}

bool readSource(const std::string& filename, std::string& source) {
  std::ifstream input(filename, std::ios::binary);

  if (!input.is_open()) {
    return false;
  }

  source.assign(std::istreambuf_iterator<char>(input),
                std::istreambuf_iterator<char>());
  input.close();

  return true;
}

} // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    usageHelp();
    return 1;
  }

  std::string arg = argv[1];
  if (arg == "-h" || arg == "--help") {
    usageHelp();
    return 1;
  }

  fs::path sourcePath{ arg };

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
  AsmParser parser{ source };

  try {
    parser.parseStatements();
  } catch (ParsingException& e) {
    cerr << e.what() << endl;
    return 1;
  }

  std::ofstream fout{ outPath.c_str(), std::ios::binary };
  parser.emit(fout);

  return 0;
}