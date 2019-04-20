#ifndef loxy_value_h
#define loxy_value_h

#include <map>
#include <set>
#include <string>
#include <cassert>
#include <cstring>
#include <memory>
#include "Common.h"
#include "VM.h"

namespace loxy {

class Chunk;
class Object;
class String;
class Module;
class VM;

typedef std::shared_ptr<Chunk> ChunkRef;

// Value representation.

/// ValueType - type tag for each Value.
enum class ValueType {
  Bool,
  Nil,
  Number,
  Obj,
  String,
  Module,

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
  bool isModule() const { return type == ValueType::Module; }

  inline operator bool () const {
    assert(type == ValueType::Bool);
    return as.boolean;
  }

  inline operator double () const {
    assert(type == ValueType::Number);
    return as.number;
  }

  inline operator Object* () const {
    assert(type == ValueType::Obj || type == ValueType::String || type == ValueType::Module);
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
    case ValueType::Module:
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
// managed smart pointers.

typedef uint32_t Hash;

/// class Object - based object type inherited by every Loxy object.
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

typedef std::map<String*, Value> SymbolTable;

/// class Module - each loxy file is a module.
class Module : public Object {
private:

  // the name of the module.
  String* name;

  // the chunk that contains bytecode.
  ChunkRef chunk;

  // top-level variables.
  SymbolTable globals;

  // local variables are stored directly on the stack.

public:

  Module(String* name, ChunkRef chunk)
    : name(name), chunk(chunk) {}

  ChunkRef getChunk() const { return chunk; }
  void setChunk(ChunkRef c) { chunk = c; }

  String* getName() const { return name; }
  void setName(String* n) { name = n; }

  /// getGlobal - finds a top-level variable within this module
  ///   if not found, returns false without setting [result].
  bool getGlobal(String* name, Value *result);

  /// setGlobal - sets a top-level variable within this module.
  bool setGlobal(String* name, Value value);

  std::string toString() const {
    Value name(this->name);
    return std::string("[module ") + name.toString() + "]";
  }

  static Module* create(VM &vm, String *name);
};  // class Module


} // namespace loxy

#endif