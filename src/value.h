/// value.h - defines memory representation of Loxy primitives/objects.
#ifndef loxy_value_h
#define loxy_value_h

#include <map>
#include <set>
#include <string>
#include <cassert>
#include <cstring>
#include <memory>
#include "common.h"
#include "vm.h"

namespace loxy {

class Chunk;
class LoxyObj;
class LoxyString;
class LoxyModule;

typedef LoxyObj*  LoxyObjRef;
typedef LoxyString* LoxyStringRef;
typedef LoxyModule* LoxyModuleRef;

// Value representation.

/// ValueType - type tag for each Value.
enum class ValueType {
  Bool,
  Nil,
  Number,
  Obj,

  // Used internally.
  Undef,
};

union Variant {
  Variant(double n) : number(n) {}
  Variant(bool v) : boolean(v) {}
  Variant(LoxyObjRef obj): obj(obj) {}

  bool boolean;
  double number;
  LoxyObjRef obj;
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

  inline operator bool () const {
    assert(type == ValueType::Bool);
    return as.boolean;
  }

  inline operator double () const {
    assert(type == ValueType::Number);
    return as.number;
  }

  inline operator LoxyObjRef () const {
    assert(type == ValueType::Obj);
    return as.obj;
  }

  bool operator == (const Value &other) const {
    if (type != other.type) return false;

    switch (type) {
    case ValueType::Bool:   return (bool)other == (bool)(*this);
    case ValueType::Nil:    return true;
    case ValueType::Undef:  return true;
    case ValueType::Number: return (double)other == (double)(*this);
    case ValueType::Obj:    return (LoxyObjRef)other == (LoxyObjRef)(*this);
    }
  }

  bool operator != (const Value &other) const {
    return !((*this) == other);
  }

  Value(ValueType type, Variant as) : type(type), as(as) {}

  Value(double number);
  Value(LoxyObjRef ref);
};

// Object representations.
//
// Objects are garbage collected, while resources used by compiler are
// managed smart pointers.

typedef uint32_t Hash;
typedef std::map<LoxyStringRef, Value> SymbolTable;
typedef std::map<Hash, LoxyStringRef> StringPool;

// global string pool.
StringPool stringPool;

/// class LoxyObj - based object type inherited by every Loxy object.
class LoxyObj {
public:

  bool    isDark;
  LoxyObjRef next;

  LoxyObj() : isDark(false), next(nullptr) {}

private:
  // object allocations should be prevented.
  void * operator new   (size_t) = delete;
  void * operator new[] (size_t) = delete;
  void   operator delete   (void *) = delete;
  void   operator delete[] (void*) = delete;

  // avoid copy.
  LoxyObj(const LoxyObj&) = delete;
  LoxyObj& operator=(const LoxyObj &) = delete;
};

/// LoxyString - string class.
class LoxyString : public LoxyObj {
private:

  // auto managed chars.
  std::unique_ptr<char> chars;
  int length;
  Hash hash_;

  static Hash hashString(const char *s);

public:

  LoxyString(std::unique_ptr<char> chars, int length, Hash hash) :
    chars(std::move(chars)), length(length), hash_(hash) {}

  Hash hash() const { return hash_; }

  bool operator== (const LoxyString &other) {
    // since we've interned strings, equality can be compared by 
    // identity.
    return other.chars == chars;
  }

  operator char*() { return chars.get(); }

  /// create - called by VM. creates a loxy string object. 
  ///   [chars] will not be owned by LoxyString.
  static LoxyStringRef create(LoxyVM &vm, const char *chars);
};

/// class LoxyModule - each loxy file is a module.
class LoxyModule : public LoxyObj {
private:

  // the name of the module.
  LoxyStringRef name;

  // the chunk that contains bytecode.
  ChunkRef chunk;

  // top-level variables.
  SymbolTable globals;

  // local variables are stored directly on the stack.

public:

  LoxyModule(LoxyStringRef name, ChunkRef chunk)
    : name(name), chunk(chunk) {}

  ChunkRef getChunk() const { return chunk; }
  void setChunk(ChunkRef c) { chunk = c; }

  LoxyStringRef getName() const { return name; }
  void setName(LoxyStringRef n) { name = n; }

  LoxyModule(ChunkRef chunk, LoxyStringRef name) : name(name), chunk(chunk) {}

  /// getGlobal - finds a top-level variable within this module
  ///   if not found, returns false without setting [result].
  bool getGlobal(LoxyStringRef name, Value *result);

  /// setGlobal - sets a top-level variable within this module.
  bool setGlobal(LoxyStringRef name, Value value);

  static LoxyModuleRef create(LoxyVM &vm, const char *name);
};


}

#endif