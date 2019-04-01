#ifndef loxy_chunk_h
#define loxy_chunk_h

#include <memory>
#include "memory.h"
#include "common.h"
#include "buffer.h"
#include "value.h"

namespace loxy {

class LoxyVM;

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
};

/// class Chunk - a structure contains compiled bytecode for LoxyVM.
class Chunk {
  LoxyVM  &vm;
public:
  // Our bytecode is designed to be of 1-byte length.
  uint8_t *code;
  size_t  count;
  size_t  capacity;

  // Correspondance line info in source code.
  int *lines;

  // Constant pool.
  Buffer<Value> constants;

public:
  // A relatively complicated constructor.
  Chunk(LoxyVM &vm, uint8_t *code = NULL, int *lines = NULL,
        size_t count = 0, size_t capacity = 0
  ): vm(vm), code(code),
     count(count), capacity(capacity), lines(lines),
     constants(Buffer<Value>(vm)) {}

  ~Chunk();

  /// makeChunk - creates a chunk instance.
  static std::shared_ptr<Chunk> makeChunk(LoxyVM &vm);

  /// write - writes a byte to chunk's code.
  void write(uint8_t byte, int line);
  
  /// addConstant - adds [value] to its constant pool & returns the index
  ///   of the added value in the pool.
  int addConstant(Value value);
};

};

#endif