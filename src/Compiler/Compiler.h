#ifndef loxy_compiler_h
#define loxy_compiler_h
#include <memory>

namespace loxy {

class Chunk;
class Parser;
class Module;
class VM;

// class Compiler - contains a set of methods for compiling
class Compiler {
public:

  // compiles [source] & returns a Chunk containing bytecode
  static std::unique_ptr<Chunk> compile(VM &vm, const char *source);
};

} // namespace loxy

#endif