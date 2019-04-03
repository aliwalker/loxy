/// memory.h - declares allocator for loxy.
#ifndef loxy_memory_h
#define loxy_memory_h

#include <stdlib.h>
#include "common.h"

namespace loxy {

inline size_t grow_capacity(size_t oldCap) {
  return oldCap < 8 ? 8 : (oldCap) * 2;
}

};

#endif