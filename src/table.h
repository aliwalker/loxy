/// table.h - defines a hash map.
#ifndef loxy_table_h
#define loxy_table_h

#include "common.h"
#include "value.h"

namespace loxy {

class LoxyVM;

struct Entry {
  Value key;
  Value value;
};

struct Table {
  size_t count;
  size_t capacity;
  Entry *entries;

  LoxyVM &vm;
private:
  void adjustCap(size_t capacity);

public:
  Table(LoxyVM &vm, size_t initialCap = 0);

  /// get - gets the value of the given key.
  bool get(Value &key, Value *value);

  /// set - sets a value of given key. Overwrite if [key] exists.
  bool set(Value &key, Value *value);

  /// del - deletes [key].
  bool del(Value key);

  /// addFrom - adds all entries from [source].
  void addFrom(Table &source);
};

} // namespace loxy

#endif