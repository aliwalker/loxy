#include <stdio.h>
#include <cstring>
#include "value.h"
#include "memory.h"
#include "vm.h"

namespace loxy {

// class Value
//
Value::Value(double number) : type(ValueType::Number), as(number) {}
Value::Value(LoxyObjRef ref) : type(ValueType::Obj), as(ref) {}

const Value Value::Nil(ValueType::Nil, Variant((double)0));
const Value Value::Undef(ValueType::Undef, Variant((double)0));
const Value Value::True(ValueType::Bool, Variant(true));
const Value Value::False(ValueType::Bool, Variant(false));

// class LoxyString
//
Hash LoxyString::hashString(const char *chars) {
  int length = strlen(chars);
  Hash _hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    _hash ^= chars[i];
    _hash *= 16777619;
  }

  return _hash;
}

LoxyStringRef LoxyString::create(LoxyVM &vm, const char *chars) {
  Hash hash = hashString(chars);
  auto s = stringPool.find(hash);

  // If we've interned this string, simply return it.
  if (s != stringPool.end()) {
    return s->second;
  }

  // allocate memory for this object.
  auto mem = vm.newObject(sizeof(LoxyString));
  std::unique_ptr<char> dupChars {strdup(chars)};
  auto ref = ::new(mem) LoxyString(std::move(dupChars), strlen(chars), hash);

  // put it in pool.
  stringPool[hash] = ref;
  return ref;
}

// class LoxyModule
//
bool LoxyModule::getGlobal(LoxyStringRef name, Value *result) {
  auto global = globals.find(name);

  if (global == globals.end()) {
    return false;
  }

  *result = global->second;
  return true;
}

LoxyModuleRef LoxyModule::create(LoxyVM &vm, const char *name) {
  auto modName = LoxyString::create(vm, name);
  auto chunk = Chunk::create();

  auto mem = vm.newObject(sizeof(LoxyModule));
  auto module = ::new LoxyModule(modName, chunk);

  return module;
}

} // namespace loxy