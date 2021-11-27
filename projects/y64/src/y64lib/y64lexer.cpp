#include "y64lexer.hpp"

#include <cassert>
#include <cctype>
#include <limits>
#include <regex>
#include <unordered_map>

#include "util.hpp"

namespace y64 {

void AsmLexer::advance(std::size_t n) {
  assert(curPtr + n <= endPtr);
  col += static_cast<int>(n);
  curPtr += n;
}

AsmToken::Kind AsmLexer::lookahead() {
  if (needEat && !nextTokens.empty()) {
    nextTokens.pop_back();
  }
  if (nextTokens.empty()) {
    nextTokens.push_back(lexToken());
  }

  needEat = false;
  return nextTokens.back().getKind();
}

const AsmToken &AsmLexer::lex() {
  if (needEat && !nextTokens.empty()) {
    nextTokens.pop_back();
  }
  if (nextTokens.empty()) {
    nextTokens.push_back(lexToken());
  }

  needEat = true;
  return nextTokens.back();
}

AsmToken AsmLexer::lexToken() {
  tokenStart = curPtr;

  char curChar = getNextChar();
  switch (curChar) {
  case EOF:
    return AsmToken(AsmToken::TKEOF, std::string_view(tokenStart, 0));
  case '\0':
  case ' ':
  case '\t':
    while (*curPtr == ' ' || *curPtr == '\t') {
      advance();
    }
    // skip space
    return lexToken();
  case '\r':
    if (curPtr != endPtr && *curPtr == '\n') {
      advance();
    }
    ++line;
    col = 0;
    return AsmToken(AsmToken::ENDLINE,
                    std::string_view(tokenStart, curPtr - tokenStart));
  case '\n':
    ++line;
    col = 0;
    return AsmToken(AsmToken::ENDLINE, std::string_view(tokenStart, 1));
  case '#':
    return lexComment();
  case ',':
    return AsmToken(AsmToken::COMMA, std::string_view(tokenStart, 1));
  case ':':
    return AsmToken(AsmToken::COLON, std::string_view(tokenStart, 1));
  case '%':
    return lexRegister();
  case '$':
    return AsmToken(AsmToken::DOLLAR, std::string_view(tokenStart, 1));
  case '(':
    return AsmToken(AsmToken::LPAREN, std::string_view(tokenStart, 1));
  case ')':
    return AsmToken(AsmToken::RPAREN, std::string_view(tokenStart, 1));
  case '.':
    return lexPseudoInst();
  case '-':
  case '+':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return lexNumber();
  default:
    if (std::isalpha(curChar) || curChar == '_') {
      return lexIdentifier();
    }

    parseError("%d: Unexpected symbol near '%c'", line, curChar);
    break;
  }
}

char AsmLexer::getNextChar() {
  if (curPtr == endPtr) {
    return EOF;
  }

  char nextChar = *curPtr;
  advance();
  return nextChar;
}

AsmToken AsmLexer::lexComment() {
  while (*curPtr != EOF && *curPtr != '\n' && *curPtr != '\r') {
    advance();
  }

  return AsmToken(AsmToken::COMMENT,
                  std::string_view(tokenStart, curPtr - tokenStart));
}

bool AsmLexer::isDigit(char ch, int base) const {
  assert(base == 10 || base == 16);
  if (base == 10) { // dec
    return std::isdigit(ch);
  }

  // hex
  return std::isdigit(ch) || (ch >= 'A' && ch <= 'F') ||
         (ch >= 'a' && ch <= 'f');
}

AsmToken AsmLexer::lexNumber() {
  bool isNegative = false;
  if (curPtr[-1] == '-') {
    isNegative = true;
    ++curPtr;
  } else if (curPtr[-1] == '+') {
    ++curPtr;
  }

  if (curPtr == endPtr) {
    return lexDecOrHexNumber(10, isNegative);
  }

  if (curPtr[-1] == '0' && (*curPtr == 'x' || *curPtr == 'X')) {
    advance(2); // eat '0[xX]'
    return lexDecOrHexNumber(16, isNegative);
  }

  return lexDecOrHexNumber(10, isNegative);
}

AsmToken AsmLexer::lexDecOrHexNumber(int base, bool isNegative) {
  std::string numStr;
  numStr.reserve(24);

  if (isNegative) {
    numStr += '-';
  }
  numStr += curPtr[-1];

  while (isDigit(*curPtr, base)) {
    numStr += *curPtr;
    advance();
  }

  long long value = 0;

  try {
    value = std::stoll(numStr, nullptr, base);
  } catch (std::invalid_argument &) {
    parseError("%d: Invalid number '%s'", line, numStr.c_str());
  } catch (std::out_of_range &) {
    parseError("%d: Immediate '%s' is too large", line, numStr.c_str());
  }

  return AsmToken::makeNumber(tokenStart, curPtr - tokenStart,
                              static_cast<std::int64_t>(value));
}

AsmToken AsmLexer::lexRegister() {
  AsmToken token = lexIdentifier(true);
  std::string identifier{token.toStringRef().data(),
                         token.toStringRef().size()};
  static const std::regex registerRegex{
      "%rax|%rcx|%rdx|%rbx|%rsp|%rbp|%rsi|%rdi|"
      "%r8|%r9|%r10|%r11|%r12|%r13|%r14"};
  bool isMatch = std::regex_match(identifier, registerRegex,
                                  std::regex_constants::match_continuous);

  if (!isMatch) {
    parseError("%d: Unknown register name '%s'", line, identifier.c_str());
  }

  token.setKind(AsmToken::REGISTER);

  return token;
}

AsmToken AsmLexer::lexPseudoInst() {
  AsmToken token = lexIdentifier(true);
  std::string identifier{token.toStringRef().data(),
                         token.toStringRef().size()};
  static const std::regex pseudoRegex{
      "\\.byte|\\.word|\\.long|\\.quad|\\.pos|\\.align"};
  bool isMatch = std::regex_match(identifier, pseudoRegex,
                                  std::regex_constants::match_continuous);

  if (!isMatch) {
    parseError("%d: Unknown pseudo instruction name '%s'", line,
               identifier.c_str());
  }

  token.setKind(AsmToken::PSEUDO_INST);

  return token;
}

// [a-zA-Z_][a-zA-Z0-9_]*
AsmToken AsmLexer::lexIdentifier(bool hasPrefix) {
  std::string identifier;
  char firstChar = curPtr[-1];
  if (hasPrefix) {
    firstChar = getNextChar();
  }

  identifier += firstChar;

  if (!std::isalpha(firstChar) && firstChar != '_') {
    parseError("%d: unexpected character '%c'", line, firstChar);
  }

  while (std::isalnum(*curPtr) || *curPtr == '_') {
    identifier += *curPtr;
    advance();
  }

  if (hasPrefix) {
    return AsmToken(AsmToken::IDENTIFIER,
                    std::string_view(tokenStart, curPtr - tokenStart));
  }

  // try to match instruction name
  static const std::regex instRegex{
      "halt|nop|rrmovq|cmovle|cmovl|cmove|cmovne|"
      "cmovge|cmovg|rmmovq|mrmovq|irmovq|addq|subq|"
      "andq|xorq|jmp|jle|jl|je|jne|jge|jg|call|ret|"
      "pushq|popq"};

  bool isMatchInst = std::regex_match(identifier, instRegex,
                                      std::regex_constants::match_continuous);
  if (isMatchInst) {
    return AsmToken(AsmToken::INST,
                    std::string_view(tokenStart, curPtr - tokenStart));
  }

  return AsmToken(AsmToken::IDENTIFIER,
                  std::string_view(tokenStart, curPtr - tokenStart));
}

} // namespace y64