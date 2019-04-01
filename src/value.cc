#include "value.h"
#include "memory.h"
#include "vm.h"

namespace loxy {

bool Value::valuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;

  switch (a.type) {
  case ValueType::Bool :  return AS_BOOL(a) == AS_BOOL(b);
  case ValueType::Nil : return true;
  case ValueType::Number :  return AS_NUMBER(a) == AS_NUMBER(b);
  case ValueType::Obj : {
    return AS_OBJ(a) == AS_OBJ(b);
  }
  }
}

static uint32_t hashString(const char *key, int length) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= key[i];
    hash *= 16777619;
  }

  return hash;
}

static uint32_t hashObject(Obj *obj) {
  switch (obj->type) {
  // strings are hashed on creation.
  case ObjType::String :  return ((ObjString*)obj)->hash;
  default:
    ASSERT(false, "Only immutable objects can be hashed.");
    return 0;
  }
}

uint32_t Value::hashValue(Value value) {
  switch (value.type) {
  case ValueType::Obj : return hashObject(AS_OBJ(value));
  case ValueType::Nil : return 1;
  case ValueType::Bool : AS_BOOL(value) ? 2 : 0;
  default:
    ASSERT(false, "Unhashable value!");
    return 0;
  }
}

bool Obj::isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString * ObjString::from(const char *chars, int length, bool take) {
  uint32_t  hash = hashString(chars, length);
  
}

}