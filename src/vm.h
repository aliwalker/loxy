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

typedef std::shared_ptr<Compiler> CompilerRef;
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
  /// newObject - allocates memory for object of type [T]. 
  void *newObject(size_t size);

  /// run - runs currModule.
  InterpretResult run();

public:

  // a linked list of modules loaded.
  LoxyModule *currModule;

  // offset in the currModule's chunk.
  size_t offset = 0;

  // execution stack.
  std::vector<Value> stack;

  // interned strings.
  std::set<LoxyString*, bool> strings;

  // A linked-list of allocated objects.
  LoxyObjRef first = nullptr;

private:

  void runtimeError(const char *format, ...);

  // helpers for reading from currModule's chunk.
  //
  uint8_t readByte();
  Value readConstant();
  LoxyStringRef readString();

  Value peek(int distance);
};

} // namespace loxy

#endif