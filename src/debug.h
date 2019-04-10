#ifndef loxy_debug_h
#define loxy_debug_h

#include "common.h"

namespace loxy {

class Chunk;

class ChunkPrinter {
public:
  static void printChunk(Chunk &chunk, const char *name);

private:

  static size_t printInstruction(Chunk &chunk, size_t offset);

  // prints instruction with 1 arg = constant.
  static size_t constantInstruction(const char *Inst, Chunk &chunk, size_t offset);

  // prints instruction that has no arg.
  static size_t simpleInstruction(const char *Ints, Chunk &chunk, size_t offset);

  // prints instruction that has 1 arg.
  static size_t byteInstruction(const char *Inst, Chunk &chunk, size_t offset);
};  // class ChunkPrinter

} // namespace loxy

#endif