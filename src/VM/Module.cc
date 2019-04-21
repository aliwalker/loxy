#include "Compiler/Compiler.h"
#include "Module.h"
#include "Value.h"
#include "VM.h"

namespace loxy {

Module *Module::create(VM &vm, String *name, String *path, String *src) {
  void *mem = vm.reallocate(nullptr, 0, sizeof(Module));
  auto imports = SmallVector<Module*>::create(vm);

  assert(mem != nullptr && "Out of memory");
  return ::new Module(vm, name, path, src, imports);
}

void Module::destroy(VM &vm, Module **module) {
  if (*module == nullptr) return;

  // free owned resources
  SmallVector<Module*>::destroy(vm, &((*module)->imports_));
  (*module)->imports_ = nullptr;

  // free itself
  vm.reallocate(*module, sizeof(Module), 0);
  *module = nullptr;
}

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

bool Module::compile() {
  assert(src_ != nullptr && "Source code can't be NULL");

  auto chunk = Compiler::compile(vm, src_->cString());
  if (chunk == nullptr) {
    return false;
  }
  bytecode_ = std::move(chunk);
  return true;
}

} // namespace loxy