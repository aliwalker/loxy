#ifndef loxy_vm_h
#define loxy_vm_h

#include "common.h"

namespace loxy {

class LoxyVM {
public:
  size_t  allocatedBytes;
  size_t  nextGC;
};

}

#endif