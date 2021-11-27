#ifndef Y64_LIB_ASMTOKEN_HPP
#define Y64_LIB_ASMTOKEN_HPP

#include <string>
#include <string_view>

namespace y64 {

class AsmToken {
public:
  enum Kind {
    TKEOF,
    ERROR,
    ENDLINE,
    IDENTIFIER,
    NUMBER,
    REGISTER,
    PSEUDO_INST,
    INST,
    COMMA,   // ,
    COMMENT, // #
    COLON,   // :
    DOLLAR,  // $
    LPAREN,  // (
    RPAREN,  // )
  };

  AsmToken() : kind(ERROR), tokenStr(), value(0) {}
  AsmToken(Kind kind, std::string_view str)
      : kind(kind), tokenStr(str), value(0) {}

  static AsmToken makeNumber(const char *start, std::size_t len,
                             std::int64_t val) {
    AsmToken token{NUMBER, std::string_view{start, len}};
    token.value = val;
    return token;
  }

  AsmToken(const AsmToken &) = default;
  AsmToken(AsmToken &&) noexcept = default;
  AsmToken &operator=(const AsmToken &) = default;
  AsmToken &operator=(AsmToken &&) noexcept = default;

  Kind getKind() const { return kind; }

  std::string_view toStringRef() const { return tokenStr; }

  std::string toString() const {
    return std::string(tokenStr.data(), tokenStr.size());
  }

  const char *data() const { return tokenStr.data(); }

  void setKind(Kind k) { kind = k; }

  std::int64_t getValue() const { return value; }

  const char *kindString() const { return kindToString(kind); }

  bool isError() const { return kind == ERROR; }

  static const char *kindToString(AsmToken::Kind kind) {
    switch (kind) {
    case y64::AsmToken::TKEOF:
      return "eof";
    case y64::AsmToken::ERROR:
      return "error";
    case y64::AsmToken::ENDLINE:
      return "endline";
    case y64::AsmToken::IDENTIFIER:
      return "identifier";
    case y64::AsmToken::NUMBER:
      return "number";
    case y64::AsmToken::REGISTER:
      return "register";
    case y64::AsmToken::PSEUDO_INST:
      return "pseudo instruction";
    case y64::AsmToken::INST:
      return "instruction";
    case y64::AsmToken::COMMA:
      return ",";
    case y64::AsmToken::COMMENT:
      return "#comment";
    case y64::AsmToken::COLON:
      return ":";
    case y64::AsmToken::DOLLAR:
      return "$";
    case y64::AsmToken::LPAREN:
      return "(";
    case y64::AsmToken::RPAREN:
      return ")";
    default:
      return "";
    }
  }

private:
  Kind kind;
  std::string_view tokenStr;
  std::int64_t value;
};

} // namespace y64

#endif // !Y64_LIB_ASMTOKEN_HPP