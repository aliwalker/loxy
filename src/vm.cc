#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "value.h"
#include "vm.h"

namespace loxy {

InterpretResult LoxyVM::interpret(const char *source, const char *module) {
  auto prevComp = compiler;
  compiler = Compiler::create(*this);
  
  auto mod = LoxyModule::create(*this, module);

  mod->next = currModule;
  currModule = mod;

  if (!(compiler->compile(source, *mod))) {
    currModule = (LoxyModuleRef)currModule->next;
    // mod->free();
    return InterpretResult::Compile_Error;
  }

  return run();
}

void *LoxyVM::newObject(size_t size) {
  LoxyObjRef obj = reinterpret_cast<LoxyObjRef>(realloc(nullptr, size));

  allocatedBytes += size;
  obj->next = first;
  first = obj;

  return reinterpret_cast<void*>(obj);
}

void LoxyVM::runtimeError(const char *format, ...) {

}

} // namespace loxy