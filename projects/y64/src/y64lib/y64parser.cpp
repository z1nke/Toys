#include "y64parser.hpp"

#include <cassert>
#include <cinttypes>
#include <unordered_map>

#include "util.hpp"

using namespace std::string_view_literals;

namespace {

constexpr bool
stringViewStartsWith(std::string_view sv, std::string_view str) {
  return sv.size() >= str.size() && 
      std::string_view::traits_type::compare(
          sv.data(), str.data(), str.size()) == 0;
}

} // namespace

namespace y64 {

static std::unordered_map<std::string/*label*/,
                          std::uint64_t/*address*/> labelTable;

static std::unordered_map<std::uint64_t/*pending addr*/,
                          std::string/*label*/> pendingAddr2Label;

std::uint64_t AsmParser::pendingAddress = 1;

// statement := label | instruction | pseudo_instruction
// label := identifier:
// instruction := inst operands
// pseudo_instruction := pseudo_inst operands
void AsmParser::parseStatements() {
  AsmToken::Kind nextKind = lexer.lookahead();

  std::vector<Instruction> insts;

  while (true) {
    switch (nextKind) {
    case AsmToken::TKEOF:
      calcPendingAddress(insts);
      genAllCode(insts);
      return;
    case AsmToken::ERROR:
      parseError("%d: Unknown token", lexer.getLine());
    case AsmToken::IDENTIFIER:
      parseLabel(insts);
      break;
    case AsmToken::INST:
      insts.push_back(parseInstruction());
      break;
    case AsmToken::PSEUDO_INST:
      insts.push_back(parseDirective());
      break;
    case AsmToken::ENDLINE:
      Y64_FALLTHROUGH;
    case AsmToken::COMMENT:
      lexer.lex(); // eat comment
      break;
    default: {
      int line = lexer.getLine();
      const AsmToken& token = lexer.lex();
      parseError("%d: Unexpected token '%s'", line,
            token.toString().c_str());
    }
    }

    nextKind = lexer.lookahead();
  }
}

void AsmParser::calcPendingAddress(std::vector<Instruction>& insts) {
  for (Instruction& inst : insts) {
    if (!inst.isPendingAddress) {
      continue;
    }

    std::uint64_t dummyAddr = static_cast<std::uint64_t>(inst.value);
    const std::string& label = pendingAddr2Label[dummyAddr];
    if (label.empty()) {
      parseError("Unknown dummy address '%" PRIu64 "'", dummyAddr);
    }

    auto iter = labelTable.find(label);
    if (iter == labelTable.end()) {
      parseError("%d: Unknown label name '%s'", inst.line, label.c_str());
    }

    inst.value = static_cast<std::int64_t>(iter->second);
    inst.isPendingAddress = false;
  }
}

void AsmParser::genAllCode(const std::vector<Instruction>& insts) {
  for (const Instruction& inst : insts) {
    genBinary(inst);
  }
}

void AsmParser::emit(std::ofstream& fout) {
  assert(fout.is_open());
  fout.write(reinterpret_cast<const char*>(out.data()), out.size());
}

#define END_INSTRUCTION                               \
    if (lexer.lookahead() == AsmToken::COMMENT) {     \
       lexer.lex();                                   \
    }                                                 \
    assertNextToken(AsmToken::ENDLINE, line, true);   \
    inst.setAddress(curPos);                          \
    curPos += inst.length();                          \
    return inst;


Instruction AsmParser::parseInstruction() {
  AsmToken instToken = lexer.lex();
  int line = lexer.getLine();

  Instruction inst;
  inst.line = line;
  inst.isPseduo = false;
  inst.isPendingAddress = false;

  static std::uint64_t pendingAddr = 1;

  std::string_view instOpCode = instToken.toStringRef();
  if (instOpCode == "halt"sv) {
    inst.setOpCode(Instruction::halt);
    END_INSTRUCTION;
  }

  if (instOpCode == "nop"sv) {
    inst.setOpCode(Instruction::nop);
    END_INSTRUCTION;
  }

  if (instOpCode == "rrmovq"sv) {
    inst.setOpCode(Instruction::rrmovq);
    parseRR(inst);
    END_INSTRUCTION;
  }

  if (stringViewStartsWith(instOpCode, "cmov"sv)) {
    inst.icode = Instruction::icode_cmov;
    inst.ifun = getCondIFun(instOpCode);
    parseRR(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "irmovq"sv) {
    inst.setOpCode(Instruction::irmovq);
    parseImmediate(inst);
    assertNextToken(AsmToken::COMMA, line, true);
    inst.regA = Register::makeNone();
    parseRegister(inst, kRight);
    END_INSTRUCTION;
  }

  if (instOpCode == "rmmovq"sv) {
    // rmmovq rA, D(rB)
    inst.setOpCode(Instruction::rmmovq);
    parseRegister(inst, kLeft);
    assertNextToken(AsmToken::COMMA, line, true);
    parseMemory(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "mrmovq"sv) {
    // mrmovq D(rB), rA
    inst.setOpCode(Instruction::mrmovq);
    parseMemory(inst);
    assertNextToken(AsmToken::COMMA, line, true);
    parseRegister(inst, kLeft);
    END_INSTRUCTION;
  }

  if (instOpCode == "addq"sv) {
    inst.setOpCode(Instruction::addq);
    parseRR(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "subq"sv) {
    inst.setOpCode(Instruction::subq);
    parseRR(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "andq"sv) {
    inst.setOpCode(Instruction::andq);
    parseRR(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "xorq"sv) {
    inst.setOpCode(Instruction::xorq);
    parseRR(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "jmp"sv) {
    inst.setOpCode(Instruction::jmp);
    parseImmediate(inst);
    END_INSTRUCTION;
  }

  if (stringViewStartsWith(instOpCode, "j"sv)) {
    inst.icode = Instruction::icode_j;
    inst.ifun = getCondIFun(instOpCode);
    parseImmediate(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "call"sv) {
    inst.setOpCode(Instruction::call);
    parseImmediate(inst);
    END_INSTRUCTION;
  }

  if (instOpCode == "ret"sv) {
    inst.setOpCode(Instruction::ret);
    END_INSTRUCTION;
  }

  if (instOpCode == "pushq"sv) {
    inst.setOpCode(Instruction::pushq);
    parseRegister(inst, kLeft);
    inst.regB = Register::makeNone();
    END_INSTRUCTION;
  }

  if (instOpCode == "popq"sv) {
    inst.setOpCode(Instruction::popq);
    parseRegister(inst, kLeft);
    inst.regB = Register::makeNone();
    END_INSTRUCTION;
  }

  parseError("%d: Unknown instruction '%s'", line, 
        std::string(instOpCode.data(), instOpCode.size()).c_str());
}

Instruction AsmParser::parseDirective() {
  AsmToken directiveToken = lexer.lex();
  int line = lexer.getLine();

  Instruction inst;
  inst.line = line;
  inst.isPseduo = true;
  inst.isPendingAddress = false;

  if (directiveToken.toStringRef() == ".pos"sv) {
    inst.setOpCode(Instruction::dot_pos);
    assertNextToken(AsmToken::NUMBER, line, false, ".pos");
    AsmToken addrToken = lexer.lex();

    inst.value = addrToken.getValue();
    assertNextToken(AsmToken::ENDLINE, line, true);
    curPos = inst.value;
    return inst;
  }

  if (directiveToken.toStringRef() == ".align"sv) {
    inst.setOpCode(Instruction::dot_align);

    assertNextToken(AsmToken::NUMBER, line, false, ".align");
    AsmToken alignToken = lexer.lex();
    
    std::int64_t alignValue = alignToken.getValue();
    if (alignValue != 1 && alignValue != 2 &&
        alignValue != 4 && alignValue != 8) {
      parseError("%d: Expected 1, 2, 4 or 8 alignment value");
    }
    inst.value = alignValue;
    assertNextToken(AsmToken::ENDLINE, line, true);
    curAlign = alignValue;

    return inst;
  }
  
  if (directiveToken.toStringRef() == ".quad"sv) {
    inst.setOpCode(Instruction::dot_quad);
    assertNextToken(AsmToken::NUMBER, line, false, ".quad");
    AsmToken quadToken = lexer.lex();
    
    inst.value = quadToken.getValue();
    assertNextToken(AsmToken::ENDLINE, line, true);
    
    std::uint64_t nextAddr = nextQuadAlignAddress();
    inst.setAddress(nextAddr);
    curPos = nextAddr + 8;
    return inst;
  }

  parseError("%d: Unknown pseudo instruction '%s'", line,
        directiveToken.toString().c_str());
}

std::string AsmParser::parseLabelName() {
  int line = lexer.getLine();
  AsmToken labelToken = lexer.lex();
  assert(labelToken.getKind() == AsmToken::IDENTIFIER);

  std::string labelStr = labelToken.toString();
  assertNextToken(AsmToken::COLON, line, true, labelStr.c_str());

  return labelStr;
}

void AsmParser::parseLabel(std::vector<Instruction>& insts) {
  std::vector<std::string> labels;
  labels.push_back(parseLabelName());

  // handle label
  AsmToken::Kind nextKind = lexer.lookahead();
  int line = lexer.getLine();
  std::uint64_t addr = curPos;
  while (true) {
    switch (nextKind) {
    case AsmToken::TKEOF:
      setLabelsAddress(labels, addr);
      return;
    case AsmToken::ERROR:
      parseError("%d: Unknown token", lexer.getLine());
    case AsmToken::IDENTIFIER:
      labels.push_back(parseLabelName());
      break;
    case AsmToken::INST: {
      Instruction inst = parseInstruction();
      insts.push_back(inst);
      setLabelsAddress(labels, inst.addr);
      return;
    }
    case AsmToken::PSEUDO_INST: {
      Instruction inst = parseDirective();
      insts.push_back(inst);
      if (inst.hasAddr) {
        setLabelsAddress(labels, inst.addr);
        return;
      }
      break;
    }
      
    case AsmToken::ENDLINE:
      Y64_FALLTHROUGH;
    case AsmToken::COMMENT:
      lexer.lex(); // eat comment
      break;
    default:
      parseError("%d: Unexpected token '%s'", line,
        lexer.lex().toString().c_str());
    }

    nextKind = lexer.lookahead();
  }
}

void AsmParser::setLabelsAddress(const std::vector<std::string>& labels,
                                 std::uint64_t addr) {
  for (const std::string& label : labels) {
    labelTable[label] = addr;
  }
}

void AsmParser::genBinary(const Instruction& inst) {
  // address: instruction|data\n
  if (inst.isPseduo) {
    handlePseudoInstruction(inst);
    return;
  }

  genAddress(inst.addr);

  InstBuffer buf;
  std::size_t len = 0;
  inst.emit(buf, len);
  for (std::size_t i = 0; i < len; ++i) {
    out.push_back(buf.data()[i]);
  }
  out.push_back('\n');
}

void AsmParser::handlePseudoInstruction(const Instruction& inst) {
  if (inst.getOpCode() != Instruction::dot_quad) {
    return;
  }

  // address: quad_data\n
  genAddress(inst.addr);
  genValue64(static_cast<std::uint64_t>(inst.value));
  out.push_back('\n');
}

void AsmParser::genValue64(std::uint64_t value) {
  InstBuffer buf;
  buf.append(value);

  const std::array<std::uint8_t, kMaxInstLen>& data = buf.data();
  for (std::size_t i = 0; i < buf.size(); ++i) {
    out.push_back(data[i]);
  }
}

void AsmParser::genAddress(std::uint64_t addr) {
  // generate 'address:', address is 0x...
  out.push_back('0');
  out.push_back('x');

  genValue64(addr);
  out.push_back(':');
  out.push_back(' ');
}

std::uint64_t AsmParser::nextQuadAlignAddress() {
  if (curPos % curAlign == 0) {
    return curPos;
  }

  return (curPos / curAlign + 1) * curAlign;
}

void AsmParser::assertNextToken(AsmToken::Kind expectedKind, int line,
                                bool consume, const char* before) {
  AsmToken::Kind nextKind = lexer.lookahead();
  if (consume) {
    lexer.lex();
  }
  
  if (nextKind != expectedKind) {
    if (!before) {
      parseError("%d: Expect '%s' token", line,
            AsmToken::kindToString(expectedKind));
    }

    parseError("%d: Expect '%s' token after '%s'", line,
          AsmToken::kindToString(expectedKind), before);
  }
}

std::uint8_t AsmParser::getCondIFun(std::string_view sv) {
  assert(sv.size() >= 2);
  char last = sv.back();
  switch (last) {
  case 'l':
    return Instruction::ifun_l;
  case 'g':
    return Instruction::ifun_g;
  default:
    break;
  }

  assert(last == 'e');

  char secondLast = sv.data()[sv.size() - 2];
  switch (secondLast) {
  case 'l':
    return Instruction::ifun_le;
  case 'g':
    return Instruction::ifun_ge;
  case 'n':
    return Instruction::ifun_ne;
  default:
    return Instruction::ifun_e;
  }
}

void AsmParser::parseRegister(Instruction& inst, bool isLeft) {
  assertNextToken(AsmToken::REGISTER, inst.line, false);
  AsmToken regToken = lexer.lex();
  Register reg = Register::make(inst.line, regToken.toString());

  if (isLeft) {
    inst.regA = reg;
  } else {
    inst.regB = reg;
  }
}

void AsmParser::parseImmediate(Instruction& inst) {
  AsmToken immToken = lexer.lex();
  if (immToken.getKind() == AsmToken::DOLLAR) {
    assertNextToken(AsmToken::NUMBER, inst.line, false, "$");
    immToken = lexer.lex(); // eat '$'
    inst.value = immToken.getValue();
  } else if (immToken.getKind() == AsmToken::IDENTIFIER) {
    inst.isPendingAddress = true;
    inst.value = pendingAddress;
    pendingAddr2Label[pendingAddress] = immToken.toString();
    ++pendingAddress;
  } else {
    parseError("%d: Expected immediate number or label name", inst.line);
  }
}

void AsmParser::parseMemory(Instruction& inst) {
  inst.value = 0;
  AsmToken::Kind nextKind = lexer.lookahead();
  if (nextKind == AsmToken::NUMBER) {
    AsmToken numToken = lexer.lex();
    inst.value = numToken.getValue();
  }

  nextKind = lexer.lookahead();
  assertNextToken(AsmToken::LPAREN, inst.line, true);
  parseRegister(inst, kRight);
  assertNextToken(AsmToken::RPAREN, inst.line, true);
}

void AsmParser::parseRR(Instruction& inst) {
  parseRegister(inst, kLeft);
  assertNextToken(AsmToken::COMMA, inst.line, true);
  parseRegister(inst, kRight);
}

} // namespace y64