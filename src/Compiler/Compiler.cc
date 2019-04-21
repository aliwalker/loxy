#include "Compiler.h"
#include "Parser.h"
#include "VM/VM.h"
#include "Module.h"

namespace loxy {

std::unique_ptr<Chunk> Compiler::compile(VM &vm, const char *source) {
  Parser parser(vm);
  std::unique_ptr<Chunk> currentChunk = std::make_unique<Chunk>();

  if (parser.parse(currentChunk.get(), source)) {
    return currentChunk;
  }

  return nullptr;
}

} // namespace loxy