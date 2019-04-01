/// memory.h - declares allocator for loxy.
#ifndef loxy_memory_h
#define loxy_memory_h

#include <stdlib.h>
#include "common.h"

namespace loxy {

class LoxyVM;
class LoxObj;

inline size_t grow_capacity(size_t oldCap) {
  return oldCap < 8 ? 8 : (oldCap) * 2;
}

/// Type of the reallocate function.
typedef void*(*ReallocateFn)(LoxyVM &vm, void *prev, size_t oldCap, size_t newCap);

/// LoxyReallocate - gabage collected allocator.
void *LoxyReallocate(LoxyVM &vm, void *prev, size_t oldSize, size_t newSize);

/// collectGarbage - triggers a garbage collection pass.
void collectGarbage();

};

#endif