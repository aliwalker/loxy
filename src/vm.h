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

class Chunk;
class Compiler;
class Value;
class LoxyObj;
class LoxyString;
class LoxyModule;

typedef LoxyObj*  LoxyObjRef;
typedef LoxyString* LoxyStringRef;
typedef LoxyModule* LoxyModuleRef;
typedef std::shared_ptr<Chunk> ChunkRef;

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
  template<typename T, typename TRef>
  TRef newObject();

public:

  // currently compiling compiler.
  std::shared_ptr<Compiler> compiler = NULL;

  // loaded modules.
  std::vector<LoxyModule*> modules;

  // module that's currently executing.
  LoxyModule *currModule;

  // offset in the currModule's chunk.
  size_t offset = 0;

  // execution stack.
  std::vector<Value> stack;

  // interned strings.
  std::set<LoxyString*, bool> strings;

  // A linked-list of allocated objects.
  LoxyObjRef first = NULL;

private:

  void runtimeError(const char *format, ...);

  uint8_t readByte();
  Value readConstant();
  LoxyString *readString();
};

template<typename T, typename TRef>
TRef LoxyVM::newObject() {
  static_assert(std::is_base_of<LoxyObj, T>::value, "type for creation isn't derived from LoxyObj");

  size_t size = sizeof(T);
  TRef ref = (TRef)realloc(NULL, size);

  allocatedBytes += size;
  ref->next = first;
  first = ref;
  return ref;
}

} // namespace loxy

#endif