#ifndef loxy_vm_h
#define loxy_vm_h

#include <vector>
#include "Common.h"
#include "Chunk.h"
#include "Value.h"

namespace loxy {

class Chunk;
class Value;
class Object;
class String;
class Module;

enum class InterpretResult {
  Ok,
  Compile_Error,
  Runtime_Error,
};

class VM {
private:
  size_t allocatedBytes;
  size_t nextGC;

  Module *currentModule;
  size_t offset;

  std::vector<Value> stack;
  Object *first;

public:
  /// Interpret - interprets the [source] code, in the context of [module].
  InterpretResult interpret(const char *source, const char *module);

  /// newObject - allocates memory for object of type [T]. 
  void *newObject(size_t size);

  /// run - runs currModule.
  InterpretResult run();

  /// loadModule - loads a module of [name].
  Module *loadModule(const char *name);

private:

  void runtimeError(const char *format, ...);
};

} // namespace loxy

#endif