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
  compiler = std::make_shared<Compiler>(*this);

  
  
}

void LoxyVM::runtimeError(const char *format, ...) {

}

} // namespace loxy