#ifndef loxy_value_h
#define loxy_value_h

#include <map>
#include <set>
#include <string>
#include <cstring>
#include <memory>
#include "Common.h"
#include "Managed.h"

namespace loxy {

class Managed;
class Object;
class String;
class Module;
class VM;

// Value representation.

// ValueType - type tag for each Value.
enum class ValueType {
  // Used internally.
  Undef,

  Bool,
  Nil,
  Number,
  Obj,
  String,
};

union Variant {
  Variant(double n) : number(n) {}
  Variant(bool v) : boolean(v) {}
  Variant(Object *obj): obj(obj) {}
  Variant() : obj(nullptr) {}

  bool boolean;
  double number;
  Object* obj;
};

class Value {
private:

  ValueType type;
  Variant as;

public:
  Value() {}
  Value(ValueType type, Variant as) : type(type), as(as) {}
  Value(double number);
  Value(Object *ref, ValueType type = ValueType::Obj);

  static const Value Nil;
  static const Value Undef;
  static const Value True;
  static const Value False;

  const char *cString() const;

  uint32_t hash();

  // helpers for determining [value] type.
  bool isBool()   const { return type == ValueType::Bool; }
  bool isNil()    const { return type == ValueType::Nil; }
  bool isUndef()  const { return type == ValueType::Undef; }
  bool isNumber() const { return type == ValueType::Number; }
  bool isObj()    const { return type == ValueType::Obj; }
  bool isString() const { return type == ValueType::String; }

  inline operator bool () const {
    assert(type == ValueType::Bool);
    return as.boolean;
  }

  inline operator double () const {
    assert(type == ValueType::Number);
    return as.number;
  }

  inline operator Object* () const {
    assert(type == ValueType::Obj || type == ValueType::String);
    return as.obj;
  }

  operator String* () const;

  bool operator == (const Value &other) const {
    if (type != other.type) return false;

    switch (type) {
    case ValueType::Bool:   return (bool)other == (bool)(*this);
    case ValueType::Nil:    return true;
    case ValueType::Undef:  return true;
    case ValueType::Number: return (double)other == (double)(*this);

    case ValueType::String:
    case ValueType::Obj:    return (Object*)other == (Object*)(*this);
    }
  }

  bool operator > (const Value &other) const {
    assert(type == ValueType::Number && other.type == ValueType::Number && "Ordering on non number values");
    return (double)(*this) > (double)other;
  }

  bool operator != (const Value &other) const {
    return !((*this) == other);
  }
};

// Object representations.
//
class Object : Managed {
public:
  virtual const char *cString() const { return "[Loxy Object]"; };
};

typedef uint32_t Hash;

/// String - string class.
class String : public Object {
private:

  // TODO: change this for consistency.
  std::unique_ptr<char> chars;
  int length;
  Hash hash_;

  String(std::unique_ptr<char> chars, int length, Hash hash) :
    chars(std::move(chars)), length(length), hash_(hash) {}

public:

  // create - called by Parser/VM to create a loxy string object.
  //  note that this function takes care of interning strings.
  static String* create(VM &vm, const char *chars, int length = -1);

  Hash hash() const { return hash_; }

  const char *cString() const { return chars.get(); }
  
  // called by [create] to figure out the hash value.
  static Hash hashString(const char *s, int length);
};  // class tring.

} // namespace loxy

#endif