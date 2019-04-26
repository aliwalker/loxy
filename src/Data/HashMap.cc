#include "HashMap.h"
#include "VM/VM.h"

namespace loxy {

HashMap *HashMap::create(VM &vm) {
  void *mem = vm.reallocate(nullptr, 0, sizeof(HashMap));

  assert(mem != nullptr && "Out of memory!");
  return ::new(mem) HashMap(vm);
}

void HashMap::destroy(VM &vm, HashMap **mapPtr) {
  HashMap *map = *mapPtr;
  if (map == nullptr)  return;

  // free owned resource.
  vm.reallocate(map->entries_, (map->capacityMask_ + 1) * sizeof(Entry), 0);

  // free itself.
  vm.reallocate(map, sizeof(HashMap), 0);
  map = nullptr;
}

Entry *HashMap::_find(String *key) const {
  Hash index = key->hash() & capacityMask_;
  Entry *tombstone = nullptr;

  while (true) {
    Entry *entry = &entries_[index];
    if (isEmpty(entry)) return tombstone != nullptr ? tombstone : entry;
    if (isTombstone(entry)) tombstone = entry;
    if (entry->key == key)  return entry; // found entry

    index = (index + 1) & capacityMask_;
  }
}

bool HashMap::del(String *key) {
  if (count_ == 0) return false;

  Entry *entry = _find(key);
  
  // empty or tombstone entry
  if (entry->key == nullptr)  return false;

  // set tombstone
  entry->key = nullptr;
  entry->value = Value::True;
  return true;
}

bool HashMap::get(String *key, Value *value) const {
  if (entries_ == nullptr) return false;

  Entry *entry = _find(key);

  // empty or tombstone entry
  if (entry->key == nullptr) return false;

  *value = entry->value;
  return true;
}

bool HashMap::set(String *key, Value value) {
  ensureCapacity(count_ + 1);

  Entry *entry = _find(key);
  bool isNewKey = entry->key == nullptr;

  if (isEmpty(entry)) count_++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}

// [desiredCapMask] will always be 8n - 1.
void HashMap::ensureCapacity(int leastCap) {
  // enough memory for now
  if (capacityMask_ + 1 > leastCap) return;

  // grow.
  int capacityMask = growCapacity(capacityMask_ + 1);
  while (capacityMask + 1 < leastCap) capacityMask = growCapacity(capacityMask + 1) - 1;

  Entry *entries = (Entry*)vm.reallocate(nullptr, 0, (capacityMask + 1) * sizeof(Entry));
  for (int i = 0; i <= capacityMask; i++) {
    // initialization
    entries[i] = Entry();
  }

  // filter tombstone
  count_ = 0;
  for (int i = 0; i <= capacityMask_; i++) {
    Entry *entry = &entries_[i];
    if (entry->key == nullptr) continue;

    Entry *dest = _find(entry->key);
    dest->key = entry->key;
    dest->value = entry->value;

    count_++;
  }

  // free old entries
  vm.reallocate(entries_, (capacityMask_ + 1) * sizeof(Entry), 0);
  entries_ = entries;
  capacityMask_ = capacityMask;
}

} // namespace loxy
