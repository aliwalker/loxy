/// value.h - defines memory representation of Loxy primitives/objects.
#ifndef loxy_value_h
#define loxy_value_h

#include <map>
#include <set>
#include <string>
#include <cassert>
#include <cstring>
#include "chunk.h"
#include "common.h"
#include "vm.h"

namespace loxy {

class LoxyObj;
class LoxyString;

typedef LoxyObj*  LoxyRef;

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
  Variant(LoxyRef obj): obj(obj) {}

  bool boolean;
  double number;
  LoxyRef obj;
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
  bool isNumber() const { return type == ValueType::Number; }
  bool isObj()    const { return type == ValueType::Obj; }
  bool isUndef()  const { return type == ValueType::Undef; }

  inline operator bool () const {
    assert(type == ValueType::Bool);
    return as.boolean;
  }

  inline operator double () const {
    assert(type == ValueType::Number);
    return as.number;
  }

  inline operator LoxyRef () const {
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
    case ValueType::Obj:    return (LoxyRef)other == (LoxyRef)(*this);
    }
  }

  bool operator != (const Value &other) const {
    return !((*this) == other);
  }

  Value(ValueType type, Variant as) : type(type), as(as) {}

  Value(double number);
  Value(LoxyRef ref);
};

// Object representations.
//

typedef uint32_t Hash;
typedef uint8_t* IPPtr;
typedef std::map<LoxyString*, Value> SymbolTable;
typedef std::map<Hash, LoxyString*> StringPool;

// global string pool.
StringPool stringPool;

/// class LoxyObj - based object type inherited by every Loxy object.
class LoxyObj {
public:

  bool    isDark;
  LoxyRef next;

  LoxyObj() : isDark(false), next(NULL) {}
};

/// LoxyString - string class.
class LoxyString : public LoxyObj {
private:

  const char *chars;
  int length;
  Hash _hash;

  static Hash hashString(const char *s);

public:
  LoxyString(const char *chars, int length, Hash hash) :
    chars(chars), length(length), _hash(hash) {}

  Hash hash() const { return _hash; }

  /// create - creates a loxy string object. if [take] is set to true,
  ///   then [chars] will be owned by LoxyString instance.
  static LoxyString *create(LoxyVM &vm, const char *chars, bool take = false);
};

/// class LoxyModule - each loxy file is a module.
class LoxyModule : public LoxyObj {
private:

  // the name of the module.
  LoxyString *name;

  // the chunk that contains bytecode.
  Chunk *chunk;

  // top-level variables.
  SymbolTable globals;

  /// local variables are stored directly on the stack.

public:

  // next instruction to be read.
  IPPtr ip;

  LoxyModule(Chunk *chunk, LoxyString *name) : name(name), chunk(chunk) {}

  /// getGlobal - finds a top-level variable within this module
  ///   if not found, returns false without setting [result].
  bool getGlobal(LoxyString *name, Value *result);
};


};

#endif