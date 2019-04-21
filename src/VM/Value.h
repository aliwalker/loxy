#ifndef loxy_value_h
#define loxy_value_h

#include <map>
#include <set>
#include <string>
#include <cassert>
#include <cstring>
#include <memory>
#include "Common.h"

namespace loxy {

class Object;
class String;
class Module;
class VM;

// Value representation.

// ValueType - type tag for each Value.
enum class ValueType {
  Bool,
  Nil,
  Number,
  Obj,
  String,

  // Used internally.
  Undef,
};

union Variant {
  Variant(double n) : number(n) {}
  Variant(bool v) : boolean(v) {}
  Variant(Object *obj): obj(obj) {}

  bool boolean;
  double number;
  Object* obj;
};

class Value {
private:

  ValueType type;
  Variant as;

public:

  static const Value Nil;
  static const Value Undef;
  static const Value True;
  static const Value False;

  std::string toString() const;

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
  operator Module* () const;

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

  bool operator != (const Value &other) const {
    return !((*this) == other);
  }

  Value(ValueType type, Variant as) : type(type), as(as) {}

  Value(double number);
  Value(Object *ref, ValueType type = ValueType::Obj);
};

// Object representations.
//
// Objects are garbage collected, while resources used by compiler are
// managed via smart pointers.

typedef uint32_t Hash;

/// class Object - based object.
class Object {
public:

  bool    isDark;
  Object  *next;

  Object() : isDark(false), next(nullptr) {}

  virtual std::string toString() const { return "[Object]"; }

private:
  // object allocations should be prevented.
  void * operator new   (size_t) = delete;
  void * operator new[] (size_t) = delete;
  void   operator delete   (void *) = delete;
  void   operator delete[] (void*) = delete;

  // avoid copy.
  Object(const Object&) = delete;
  Object& operator=(const Object &) = delete;
};

/// String - string class.
class String : public Object {
private:

  // auto managed chars.
  std::unique_ptr<char> chars;
  int length;
  Hash hash_;

public:

  String(std::unique_ptr<char> chars, int length, Hash hash) :
    chars(std::move(chars)), length(length), hash_(hash) {}

  Hash hash() const { return hash_; }

  std::string toString() const { return this->chars.get(); }

  // create - called by Parser/VM to create a loxy string object.
  //  note that this function takes care of interning strings.
  static String* create(VM &vm, const char *chars, int length = -1);
  
  // called by [create] to figure out the hash value.
  static Hash hashString(const char *s, int length);
};  // class tring.

} // namespace loxy

#endif