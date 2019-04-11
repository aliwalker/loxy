#include <stdio.h>
#include <cstring>
#include <string>
#include "value.h"
#include "memory.h"
#include "vm.h"

namespace loxy {

// global string pool.
StringPool stringPool;

// class Value
//
Value::Value(double number) : type(ValueType::Number), as(number) {}
Value::Value(Object* ref, ValueType type) : type(type), as(ref) {}

const Value Value::Nil(ValueType::Nil, Variant((double)0));
const Value Value::Undef(ValueType::Undef, Variant((double)0));
const Value Value::True(ValueType::Bool, Variant(true));
const Value Value::False(ValueType::Bool, Variant(false));


Value::operator String* () const {
  assert(type == ValueType::String);
  return static_cast<String*>((Object*)(*this));
}

Value::operator Module* () const {
  assert(type == ValueType::Module);
  return static_cast<Module*>((Object*)(*this));
}

std::string Value::toString() const {
  switch (type) {
  case ValueType::Bool: return bool(*this) ? std::string("true") : std::string("false");
  case ValueType::Number: return std::to_string((double)(*this));
  case ValueType::Nil: return "nil";
  case ValueType::Undef: return "undef";

  // reference types' toString is virtual.
  case ValueType::String:
  case ValueType::Module:
  case ValueType::Obj: return ((Object*)(*this))->toString();
  }
}

// class String
//
Hash String::hashString(const char *chars) {
  int length = strlen(chars);
  Hash _hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    _hash ^= chars[i];
    _hash *= 16777619;
  }

  return _hash;
}

StringRef String::create(LoxyVM &vm, const char *chars) {
  Hash hash = hashString(chars);
  auto s = stringPool.find(hash);

  // If we've interned this string, simply return it.
  if (s != stringPool.end()) {
    return s->second;
  }

  // allocate memory for this object.
  auto mem = vm.newObject(sizeof(String));
  std::unique_ptr<char> dupChars {strdup(chars)};
  auto ref = ::new(mem) String(std::move(dupChars), strlen(chars), hash);

  // put it in pool.
  stringPool[hash] = ref;
  return ref;
}

StringRef String::concat(LoxyVM &vm, StringRef another) {
  auto rawResult = (std::string(chars.get()) + std::string(another->chars.get())).c_str();

  return create(vm, rawResult);
}

// class Module
//
bool Module::getGlobal(StringRef name, Value *result) {
  auto global = globals.find(name);

  if (global == globals.end()) {
    return false;
  }

  *result = global->second;
  return true;
}

bool Module::setGlobal(StringRef name, Value value) {
  bool newKey = true;

  if (globals.find(name) == globals.end()) {
    newKey = false;
  }

  auto pair = std::make_pair(name, value);
  globals.insert(pair);
  return newKey;
}

ModuleRef Module::create(LoxyVM &vm, const char *name) {
  auto modName = String::create(vm, name);
  auto chunk = Chunk::create();

  auto mem = vm.newObject(sizeof(Module));
  auto module = ::new(mem) Module(modName, chunk);

  return module;
}

} // namespace loxy