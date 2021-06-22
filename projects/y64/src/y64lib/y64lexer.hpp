#ifndef Y64_LIB_Y64_LEXER_HPP
#define Y64_LIB_Y64_LEXER_HPP

#include <vector>

#include "asmtoken.hpp"

namespace y64 {

class AsmLexer {
public:
  AsmLexer(const std::string& source) 
      : AsmLexer(source.data(), source.data() + source.size()) { }

  AsmToken::Kind lookahead();
  const AsmToken& lex();

  int getLine() const { return line; }
  int getCol() const { return col; }

private:
  AsmLexer(const char* beginPtr, const char* endPtr) 
      : curPtr(beginPtr), tokenStart(beginPtr), endPtr(endPtr),
        nextTokens(), line(1), col(1), needEat(false) {
  }

  AsmToken lexToken();
  AsmToken lexComment();
  AsmToken lexNumber();
  AsmToken lexDecOrHexNumber(int base, bool isNegative);
  AsmToken lexRegister();
  AsmToken lexPseudoInst();
  AsmToken lexIdentifier(bool hasPrefix = false);

  char getNextChar();
  void advance(std::size_t n = 1); // advance n steps
  bool isDigit(char ch, int base) const;

private:
  const char* curPtr;
  const char* tokenStart;
  const char* endPtr;
  // cache next tokens in reverse order
  std::vector<AsmToken> nextTokens;
  int line;
  int col;
  bool needEat;
};

} // namespace y64

#endif // !Y64_LIB_Y64_LEXER_HPP