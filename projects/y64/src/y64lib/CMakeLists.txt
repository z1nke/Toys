add_library(y64 STATIC
  # Headers
  asmtoken.hpp
  buffer.hpp
  instruction.hpp
  register.hpp
  util.hpp
  y64exception.hpp
  y64lexer.hpp
  y64parser.hpp
  y64machine.hpp

  # Sources
  instruction.cpp
  insts.def
  register.cpp
  registers.def
  util.cpp
  y64lexer.cpp
  y64parser.cpp
)