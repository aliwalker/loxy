#include "memory.h"
#include "vm.h"

namespace loxy {

void *reallocate(LoxyVM &vm, void *prev, size_t oldSize, size_t newSize) {
  // Update allocated bytes on the vm.
  vm.allocatedBytes += newSize - oldSize;

  if (newSize == 0) {
    free(prev);
    return NULL;
  }

  // If we hit the threshold, trigger a gc.
  if (vm.allocatedBytes >= vm.nextGC) {
    collectGarbage();
  }

  return realloc(prev, newSize);
}

};