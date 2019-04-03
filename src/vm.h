#ifndef loxy_vm_h
#define loxy_vm_h

#include <map>
#include <set>
#include <vector>

#include "chunk.h"
#include "compiler.h"
#include "common.h"
#include "value.h"

namespace loxy {

enum class InterpretResult {
  Ok,
  Compile_Error,
  Runtime_Error,
};

/// class LoxyVM - the vm for interpreting Loxy code.
class LoxyVM {
private:

  size_t allocatedBytes;
  size_t nextGC;

public:
  /// Interpret - interprets the [source] code, in the context of [module].
  InterpretResult interpret(const char *source, const char *module);

  // gc related methods.
  //
  LoxyRef newObject(size_t size);

public:

  // currently compiling compiler.
  Compiler  *compiler;

  // loaded modules.
  std::vector<LoxyModule*> modules;

  // module that's currently executing.
  LoxyModule *currModule;

  // execution stack.
  std::vector<Value> stack;

  // interned strings.
  std::set<LoxyString*, bool> strings;
private:

  // gets next instruction pointer.
  IPPtr getIP();

  void runtimeError(const char *format, ...);

  uint8_t readByte();
  Value readConstant();
  LoxyString *readString();
};

} // namespace loxy

#endif