#ifndef loxy_vm_h
#define loxy_vm_h

#include <vector>

#include "chunk.h"
#include "compiler.h"
#include "common.h"
#include "value.h"

namespace loxy {

class Chunk;
class Compiler;
class Value;
class Object;
class String;
class Module;

typedef std::shared_ptr<Compiler> CompilerRef;
typedef Object* ObjectRef;
typedef String* StringRef;
typedef Module* ModuleRef;
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

  /// loadModule - loads a module of [name].
  Module *loadModule(const char *name);

public:

  // a linked list of modules loaded.
  Module *currModule = nullptr;

  // offset in the currModule's chunk.
  size_t offset = 0;

  // execution stack.
  std::vector<Value> stack;

  // A linked-list of allocated objects.
  ObjectRef first = nullptr;

private:

  void runtimeError(const char *format, ...);

  // helpers for reading from currModule's chunk.
  //
  uint8_t readByte();
  uint16_t readShort();
  Value readConstant();
  StringRef readString();

  Value peek(int distance);
  Value pop();
  void push(Value value);
};

} // namespace loxy

#endif