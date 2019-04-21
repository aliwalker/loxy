#ifndef loxy_chunk_h
#define loxy_chunk_h

#include <vector>
#include "Data/SmallVector.h"
#include "Common.h"
#include "OpCode.h"

namespace loxy {

class Value;
class Chunk;

typedef std::shared_ptr<Chunk> ChunkRef;

// class Chunk - a structure contains compiled bytecode for LoxyVM.
class Chunk {
public:
  // Our bytecode is designed to be of 1-byte length.
  //std::vector<uint8_t> code;
  SmallVector<uint8_t> code;

  // Correspondance line info in source code.
  //std::vector<int> lines;
  SmallVector<int> lines;

  // Constant pool.
  //std::vector<Value> constants;
  SmallVector<Value> constants;

public:
  explicit Chunk(VM &vm)
  : code(vm), lines(vm), constants(vm) {}

  /// write - writes a byte to chunk's code.
  void write(uint8_t byte, int line);

  /// clear - clears [code] & [lines].
  void clear();

  /// read - reads a piece of bytecode.
  constexpr uint8_t read(size_t offset) const { return code[offset]; }

  /// size - returns the size of the bytecode array.
  constexpr size_t size() const noexcept { return code.count(); }
  
  /// addConstant - adds [value] to its constant pool & returns the index
  ///   of it in the pool.
  int addConstant(Value value);

  /// getConstants - returns the constant value at [index].
  Value getConstant(size_t index) const;
};

} // namespace loxy

#endif