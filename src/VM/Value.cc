#include <cstring>
#include <sstream>
#include "Value.h"
#include "VM.h"

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

const char *Value::cString() const {
  switch (type) {
  case ValueType::Bool:   return bool(*this) ? "true" : "false";
  case ValueType::Number: {
    std::stringstream ss;
    ss << (double)(*this);
    return ss.str().c_str();
  }
  case ValueType::Nil:    return "nil";
  case ValueType::Undef:  return "undef";

  case ValueType::String:
  case ValueType::Obj:    return ((Object*)(*this))->cString();
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
  void *mem = vm.reallocate(nullptr, 0, sizeof(String));

  std::unique_ptr<char> rawStr(reinterpret_cast<char*>(vm.reallocate(nullptr, 0, length + 1)));
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

} // namespace loxy