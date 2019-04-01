/// value.h - defines memory representation of Loxy primitives/objects.
#ifndef loxy_value_h
#define loxy_value_h

#include "common.h"

namespace loxy {

struct Obj;
struct ObjString;

/// ValueType - type tag for each Value.
enum class ValueType {
  Bool,
  Nil,
  Number,
  Obj,

  // Used internally.
  Undef,
};

struct Value {
  ValueType type;
  union {
    bool boolean;
    double number;

    Obj *obj;
  } as;

  /// valuesEqual - checks whether [a] equals [b].
  static bool valuesEqual(Value a, Value b);

  /// hashValue - generates hash for [value]. [value] must be built-in
  ///   immutable type.
  ///   This method is modified from wren's hashValue:
  ///   https://github.com/wren-lang/wren/blob/93dac9132773c5bc0bbe92df5ccbff14da9d25a6/src/vm/wren_value.c#L424
  static uint32_t hashValue(Value value);
};

/// Macros for determining types.
#define IS_BOOL(value)    ((value).type == ValueType::Bool)
#define IS_NIL(value)     ((value).type == ValueType::Nil)
#define IS_NUMBER(value)  ((value).type == ValueType::Number)
#define IS_OBJ(value)     ((value).type == ValueType::Obj)
#define IS_UNDEF(value)   ((value).type == ValueType::Undef)

// To avoid side-effects, like `IS_STRING(pop())`.
#define IS_STRING(value)    Obj::isObjType(value, ObjType::String)

/// Macros for destructing values.
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)
#define AS_OBJ(value)     ((value).as.obj)
#define AS_STRING(value)  ((ObjString*)AS_OBJ((value)))
#define AS_CSTRING(value)   (((ObjString*)AS_OBJ(value))->chars)

/// Macros for consturcting values.
#define BOOL_VAL(value)   ((Value){ ValueType::Bool, { .boolean = value } })
#define NIL_VAL           ((Value){ ValueType::Nil, { .number = 0 } })
#define NUMBER_VAL(value) ((Value){ ValueType::Number, { .number = value } })
#define OBJ_VAL(value)    ((Value){ ValueType::Obj, { .obj = (Obj*)value } })
#define UNDEF_VAL         ((Value){ ValueType::Undef, { .number = 0 } })

// Object representations.
//
// Note that we don't use inheretance since we can't avoid dynamic cast
// in runtime. The reason for using C++ here, is to take advantage of the
// enhancement on C. This might be disapointing.

enum class ObjType {
  String,
  Module,
};

/// struct Obj - general object type, contained by each Obj*.
struct Obj {
  ObjType type;
  bool isDark;
  Obj *next;

  /// isObjType - checks if [value] contains an [Obj] of [type].
  static bool isObjType(Value value, ObjType type);

  /// printObject - prints the object [value] contains.
  static void printObject(Value value);
};

/// struct ObjString - Loxy's string object.
struct ObjString {
  Obj obj;
  size_t  length;
  uint32_t  hash;
  char    *chars;
  
  /// from - creates an ObjString from [chars]. If [take] is true, then
  ///   the method does not duplicate from [chars].
  static ObjString *from(const char *chars, int length, bool take = false);
};

/// struct ObjModule - represents loaded a Loxy's module.
struct ObjModule {
  Obj obj;

  // The name of the module.
  ObjString *name;

  
};

};

#endif