#ifndef loxy_opcode_h
#define loxy_opcode_h

#include "Common.h"

namespace loxy {

enum class OpCode: uint8_t {
  /// emits the constant on the stack.
  /// the arg is the index into constant pool of current chunk.
  /// e.g: gets the 17th constant
  /// CONSTANT 16
  CONSTANT,

  /// emits a nil/true/false value onto the stack.
  NIL,
  TRUE,
  FALSE,

  /// pops off the topmost value onto the stack.
  POP,

  /// reads a global variable from symbol table & push
  /// its value onto the stack.
  /// e.g: gets a global and pushes it.
  ///   CONSTANT 1
  ///   GET_GLOBAL
  GET_GLOBAL,

  /// sets a global variable
  SET_GLOBAL,

  /// GET_LOCAL 0
  /// the arg is the index into the stack.
  GET_LOCAL,
  SET_LOCAL,
  DEFINE_GLOBAL,
  EQUAL,
  GREATER,
  LESS,
  ADD,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  NOT,
  NEGATE,
  JUMP,
  JUMP_IF_FALSE,
  LOOP,
  PRINT,
  RETURN,
};

} // namespace loxy

#endif