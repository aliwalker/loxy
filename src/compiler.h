#ifndef loxy_compiler_h
#define loxy_compiler_h

#include "common.h"
#include "chunk.h"
#include "value.h"

namespace loxy {

class Chunk;
class Compiler;
class LoxyVM;
class Parser;
class LoxyModule;

typedef std::shared_ptr<Compiler> CompilerRef;
typedef std::unique_ptr<Parser> ParserRef;

/// class Compiler - the compiler class for Loxy.
class Compiler {
private:

  /// previous compiler.
  CompilerRef parent = nullptr;

  ParserRef parser;

public:
  Compiler(LoxyVM &vm);

  /// Compile - compiles over [source].
  bool compile(const char *source, LoxyModule &module);

  CompilerRef getParent() const { return parent; }

  void setParent(CompilerRef _compiler) { parent = _compiler; }

  static CompilerRef create(LoxyVM &vm);
};

} // namespace loxy

#endif