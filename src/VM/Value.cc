#include <cstring>
#include <string>
#include "Value.h"

namespace loxy {

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
String *String::create(VM &vm, const char *chars, int length) {
  length = length == -1 ? strlen(chars) : length;
  Hash hash = hashString(chars, length);
  String *interned = vm.findString(chars, length, hash);

  if (interned != nullptr) {
    return interned;
  }

  // create & intern this new string.

  void *mem = vm.newObject(sizeof(String));

  std::unique_ptr<char> rawStr(reinterpret_cast<char*>(vm.newObject(length + 1)));
  memcpy(rawStr.get(), chars, length);
  (rawStr.get())[length] = '\0';

  interned = ::new(mem) String(std::move(rawStr), length, hash);
  vm.addString(interned);
  return interned;
}

// length is required in case [chars] does not terminate at proper place.
Hash String::hashString(const char *chars, int length) {
  Hash hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= chars[i];
    hash *= 16777619;
  }

  return hash;
}

Module *Module::create(VM &vm, String *name) {
  // TODO:
  // should place the logics of finding loaded module here.
  void *mem = vm.newObject(sizeof(Module));
  ChunkRef chunk = Chunk::create();

  return ::new(mem) Module(name, chunk);
}

bool Module::getGlobal(String *name, Value *result) {
  auto global = globals.find(name);

  if (global == globals.end()) {
    return false;
  }

  *result = global->second;
  return true;
}

bool Module::setGlobal(String *name, Value value) {
  auto global = globals.find(name);
  if (global == globals.end()) {
    auto entry = std::make_pair(name, value);
    globals.insert(entry);
    
    // indicates a new key.
    return true;
  }

  // key exists already.
  global->second = value;
  return false;
}
} // namespace loxy