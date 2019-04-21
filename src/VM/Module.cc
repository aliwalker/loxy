#include "Compiler/Compiler.h"
#include "Module.h"
#include "Value.h"
#include "VM.h"

namespace loxy {

void Module::addVariable(String *name, Value initializer) {
  auto var = variables.find(name);
  if (var == variables.end()) {
    // new variable.
    auto entry = std::make_pair(name, initializer);
    variables.insert(entry);
  }

  // not exists yet.
  var->second = initializer;
}

bool Module::getVariable(String *name, Value *result) {
  auto var = variables.find(name);
  if (var == variables.end()) return false;

  *result = var->second;
  return true;
}

bool Module::compile(VM &vm) {
  assert(src_ != nullptr && "Source code can't be NULL");

  auto chunk = Compiler::compile(vm, src_.get());
  if (chunk == nullptr) {
    return false;
  }
  bytecode_ = std::move(chunk);
  return true;
}

} // namespace loxy