#ifndef loxy_hash_map_h
#define loxy_hash_map_h

#include "Common.h"
#include "Value.h"
#include "VM/Managed.h"

namespace loxy
{

class VM;

typedef uint32_t Hash;

struct Entry {
  Value key;
  Value value;

  // an empty entry
  Entry()
  : key(Value::Undef),
    value(Value::Nil) {}
};

class HashMap : public Managed {
private:
  VM &vm;

  // number of entries
  int count_;

  // capacity is always 8n. This mask is 8n - 1.
  int capacityMask_;

  // blob of memory holding entries
  Entry *entries_;

  static Hash hashValue(Value value);

  static bool isEmpty(Entry *entry) { return entry->key == Value::Undef && entry->value == Value::Nil; }
  static bool isTombstone(Entry *entry) { return entry->key == Value::Undef && entry->value == Value::True; }

  // find an entry from [entries]. This method will always return since
  // [key] is hashable.
  Entry *_find(Value key);

  // ensures entries has at least [desiredCap].
  void ensureCapacity(int leastCap);
  int growCapacity(int old) { return old < 8 ? 8 : old * 2; }

  HashMap(VM &vm)
  : vm(vm),
    count_(0),
    capacityMask_(-1),
    entries_(nullptr) {}

public:

  static HashMap *create(VM &vm);
  static void destroy(VM &vm, HashMap **map);

  // sets entry with [key] to a tombstone if it exists. returns true indicating success.
  bool del(Value key);
  bool get(Value key, Value *result);
  bool set(Value key, Value value);
};

} // namespace loxy


#endif