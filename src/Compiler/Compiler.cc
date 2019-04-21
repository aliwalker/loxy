#include "Compiler.h"
#include "Parser.h"
#include "VM/VM.h"
#include "Module.h"

namespace loxy {

Chunk *Compiler::compile(VM &vm, const char *source) {
  Parser parser(vm);
  Chunk *chunk = Chunk::create(vm);  

  if (parser.parse(chunk, source)) {
    return chunk;
  }

  // failed to compile, free resources.
  Chunk::destroy(vm, &chunk);
  return nullptr;
}

} // namespace loxy