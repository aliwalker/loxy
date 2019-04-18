#ifndef loxy_compiler_h
#define loxy_compiler_h

#include "common.h"
#include "chunk.h"
#include "value.h"

namespace loxy {

class Chunk;
class Compiler;
class LoxyVM;
class Module;

class Compiler {
public:

  /// compileModule - compiles [source] in the context of [module].
  static bool compileModule(LoxyVM &vm, const char *source, Module &module);
};

} // namespace loxy

#endif