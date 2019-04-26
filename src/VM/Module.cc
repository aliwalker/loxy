#include "Compiler/Compiler.h"
#include "Module.h"
#include "Value.h"
#include "VM.h"

namespace loxy {

Module *Module::create(VM &vm, String *name, String *path, String *src) {
  void *mem = vm.reallocate(nullptr, 0, sizeof(Module));
  auto imports = SmallVector<Module*>::create(vm);
  auto variables = HashMap::create(vm);

  assert(mem != nullptr && "Out of memory");
  return ::new Module(vm, name, path, src, variables, imports);
}

void Module::destroy(VM &vm, Module **modPtr) {
  Module *module = *modPtr;
  if (module == nullptr) return;

  // free owned resources
  SmallVector<Module*>::destroy(vm, &module->imports_);
  HashMap::destroy(vm, &module->variables_);

  // free itself
  vm.reallocate(module, sizeof(Module), 0);
  *modPtr = nullptr;
}

void Module::addVariable(String *name, Value initializer) {
  // top-level global variables can be redeclared.
  variables_->set(name, initializer);
}

bool Module::setVariable(String *name, Value value) {
  // if [name] is a new key, then it means [name] wasn't declared,
  // thus failed.
  return !variables_->set(name, value);
}

bool Module::getVariable(String *name, Value *result) {
  return variables_->get(name, result);
}

bool Module::compile() {
  assert(src_ != nullptr && "Source code can't be NULL");

  auto chunk = Compiler::compile(vm, src_->cString());
  if (chunk == nullptr) {
    return false;
  }
  bytecode_ = chunk;
  return true;
}

} // namespace loxy