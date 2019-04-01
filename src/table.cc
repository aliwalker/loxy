#include "table.h"
#include "value.h"
#include "memory.h"

#define TABLE_MAX_LOAD  0.75

namespace loxy {

Table::Table(LoxyVM &vm, size_t initialCap): vm(vm) {
  count = 0;
  capacity = initialCap;
  entries = NULL;
}

static Entry *findEntry(Entry *entries, size_t capacity, Value key) {
  uint32_t index = Value::hashValue(key) % capacity;
  Entry *tomstone = NULL;

  while (true) {
    Entry *entry = &entries[index];

    if (IS_UNDEF(entry->key)) {
      if (IS_NIL(entry->value)) {
        // if we've encountered a tomstone, use it.
        return tomstone != NULL ? tomstone : entry;
      } else {
        // entry->value == BOOL_VAL(true).
        if (tomstone == NULL) tomstone = entry;
      }
      // We use intern string, thus equality can be determined by pointer.
    } else if (Value::valuesEqual(entry->key, key)) {
      // Found entry.
      return entry;
    }

    index = (index + 1) % capacity;
  }
}

void Table::adjustCap(size_t newCap) {
  // create a new list of entries & initialize them.
  Entry *newEntries = static_cast<Entry*>(LoxyReallocate(vm, NULL, 0, newCap * sizeof(Entry)));
  for (int i = 0; i < newCap; i++) {
    newEntries[i].key = UNDEF_VAL;
    newEntries[i].value = NIL_VAL;
  }

  // reinsert every existing entry.
  // tomstones are not cpoied.
  count = 0;
  for (int i = 0; i < capacity; i++) {
    Entry *entry = &entries[i];
    if (IS_UNDEF(entry->key)) continue;

    Entry *dest = findEntry(newEntries, newCap, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    count++;
  }

  // free old entry list.
  LoxyReallocate(vm, entries, capacity, 0);
  entries = newEntries;
  capacity = newCap;
}

bool Table::get(Value &key, Value *value) {
  if (entries == NULL)  return false;

  Entry *entry = findEntry(entries, capacity, key);
  if (IS_UNDEF(entry->key)) return false;

  *value = entry->value;
  return true;
}

bool Table::del(Value key) {
  if (count == 0) return false;

  Entry *entry = findEntry(entries, capacity, key);

  // a tomstone entry with key = Undef, value = true.
  entry->key = UNDEF_VAL;
  entry->value = BOOL_VAL(true);

  // tomstones are treated as full bucket.
  return true;
}

} // namespace loxy