#ifndef loxy_vm_h
#define loxy_vm_h

#include <map>
#include <vector>
#include "Common.h"
#include "Chunk.h"
#include "Value.h"

namespace loxy {

template<typename T>
class SmallVector;
class HashMap;
class Chunk;
class Value;
class Object;
class String;
class Module;

typedef uint32_t Hash;

class StringPool {
  HashMap *map_;

  StringPool(HashMap *map) : map_(map) {}
public:

  static StringPool *create(VM &vm);
  static void destroy(VM &vm, StringPool **poolPtr);
  String *findString(const char *chars, int length, uint32_t hash) const;
  void addString(String *string);
};

enum class InterpretResult {
  Ok,
  Compile_Error,
  Runtime_Error,
};

class VM {
  friend class String;
  friend class Module;

private:
  size_t allocatedBytes;
  size_t nextGC;

  // TODO:
  // change this to a hashmap.
  SmallVector<Module*> *modules_;

  Object *first;

  StringPool *stringPool;

public:
  VM();
  ~VM();

  /// Interpret - interprets the [source] code, in the context of [module].
  InterpretResult interpret(const char *source, const char *module);

  // reallocate - garbage collected resources are alloacted from this
  //  method.
  void *reallocate(void *prev, size_t oldSize, size_t newSize);

  void collectGarbage();

  // run - runs [module].
  InterpretResult run(Module *module);

  // loadModule - loads a module of [name].
  Module *loadModule(const char *name);

  // findString - finds a String* from underlying string pool.
  String *findString(const char *chars, int length, uint32_t hash);

  // addString - adds the give string to string pool.
  void addString(String *string);
private:

  void error(const char *msg, int line);
};

} // namespace loxy

#endif