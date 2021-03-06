#ifndef loxy_chunk_h
#define loxy_chunk_h

#include <vector>
#include "Common.h"
#include "OpCode.h"

namespace loxy {

template<typename T>
class SmallVector;
class Value;
class Chunk;
class VM;

// class Chunk - a structure contains compiled bytecode for LoxyVM.
class Chunk {
public:
  // bytecode is designed to be of 1-byte length.
  SmallVector<uint8_t> *code_;

  // Correspondance line info in source code.
  SmallVector<int> *lines_;

  // Constant pool.
  SmallVector<Value> *constants_;

  // helpers.
  SmallVector<uint8_t> &code() const { return *code_; }
  SmallVector<int> &lines() const { return *lines_; }
  SmallVector<Value> &constants() const { return *constants_; }

private:
  explicit Chunk(VM &vm, SmallVector<uint8_t> *code, 
    SmallVector<int> *lines, SmallVector<Value> *constants)
    : code_(code), lines_(lines), constants_(constants) {}

public:

  /// write - writes a byte to chunk's code.
  void write(uint8_t byte, int line);

  /// clear - clears [code] & [lines].
  void clear();

  /// read - reads a piece of bytecode.
  uint8_t read(size_t offset) const;

  /// size - returns the size of the bytecode array.
  size_t size() const noexcept;
  
  /// addConstant - adds [value] to its constant pool & returns the index
  ///   of it in the pool.
  int addConstant(Value value);

  /// getConstants - returns the constant value at [index].
  Value getConstant(size_t index) const;

  // a convenient creator.
  static Chunk *create(VM &vm);

  static void destroy(VM &vm, Chunk **chunk);
};

void DissambleChunk(Chunk *chunk, const char *name);

} // namespace loxy

#endif