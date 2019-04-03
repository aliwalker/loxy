#ifndef loxy_compiler_h
#define loxy_compiler_h

#include "common.h"
#include "chunk.h"
#include "value.h"

namespace loxy {

class LoxyVM;
class Parser;

/// class Compiler - the compiler class for Loxy.
class Compiler {
  Parser &parser;
  LoxyVM &vm;
public:
  Compiler(LoxyVM &vm);

  /// Compile - compiles over [source].
  void Compile(const char *source, ObjModule &module);
  void markCompiler();

public:
  Chunk *compilingChunk;
};

};

#endif