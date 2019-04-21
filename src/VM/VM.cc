#include <stdlib.h>

#include "VM.h"
#include "Value.h"

namespace loxy {

void *VM::reallocate(void *prev, size_t oldSize, size_t newSize) {
  allocatedBytes += newSize - oldSize;

  // [collectGarbage] will remove this object from
  // allocated list.
  if (newSize == 0) {
    free(prev);
    return nullptr;
  }
  
  if (newSize > oldSize) {
  #ifdef DEBUG_GC
    collectGarbage();
  #endif
    if (allocatedBytes > nextGC) {
      collectGarbage();
    }
  }

  return realloc(prev, newSize);
}

} // namespace loxy