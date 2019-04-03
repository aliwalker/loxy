#ifndef loxy_chunk_h
#define loxy_chunk_h

#include <memory>
#include <vector>
#include "common.h"
#include "value.h"

namespace loxy {

enum class OpCode: uint8_t {
  CONSTANT,
  NIL,
  TRUE,
  FALSE,
  POP,
  GET_GLOBAL,
  SET_GLOBAL,
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
  PRINT,
  RETURN,
};  // enum OpCode

/// class Chunk - a structure contains compiled bytecode for LoxyVM.
class Chunk {
public:
  // Our bytecode is designed to be of 1-byte length.
  std::vector<uint8_t> code;

  // Correspondance line info in source code.
  std::vector<int> lines;

  // Constant pool.
  std::vector<Value> constants;

public:
  /// write - writes a byte to chunk's code.
  void write(uint8_t byte, int line);
  
  /// addConstant - adds [value] to its constant pool & returns the index
  ///   of the added value in the pool.
  int addConstant(Value value);
};

};

#endif