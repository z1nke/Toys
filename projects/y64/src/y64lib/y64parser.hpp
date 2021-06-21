#ifndef Y64_LIB_Y64_PARSER_HPP
#define Y64_LIB_Y64_PARSER_HPP

#include "y64lexer.hpp"
#include "instruction.hpp"

#include <fstream>

namespace y64 {

class AsmParser {
public:
  AsmParser(const std::string& source) 
      : lexer(source), curPos(0), out(), curAlign(8) { }

  void parseStatements();
  void emit(std::ofstream& fout);

private:
  Instruction parseInstruction();
  Instruction parseDirective();
  void parseLabel(std::vector<Instruction>& insts);
  std::string parseLabelName();
  void setLabelsAddress(const std::vector<std::string>& labels,
                        std::uint64_t addr);

  void assertNextToken(AsmToken::Kind expectedKind, int line,
                       bool consume = false,
                       const char* before = nullptr);
  std::uint8_t getCondIFun(std::string_view sv);
  void parseRegister(Instruction& inst, bool isLeft);
  void parseImmediate(Instruction& inst);
  void parseMemory(Instruction& inst);
  void parseRR(Instruction& inst);

  void calcPendingAddress(std::vector<Instruction>& insts);
  void genAllCode(const std::vector<Instruction>& insts);

  void genBinary(const Instruction& inst);
  void genValue64(std::uint64_t value);
  void genAddress(std::uint64_t addr);
  void handlePseudoInstruction(const Instruction& inst);
  std::uint64_t nextQuadAlignAddress();

private:
  static std::uint64_t pendingAddress;
  static const bool kLeft = true;
  static const bool kRight = false;

  AsmLexer lexer;
  std::vector<std::uint8_t> out;
  std::uint64_t curPos;
  std::size_t curAlign;
};

} // namespace y64

#endif // !Y64_LIB_Y64_PARSER_HPP