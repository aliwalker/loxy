#ifndef loxy_managed_h
#define loxy_managed_h

#include "Common.h"

namespace loxy {

/// class Managed - based object.
class Managed {
public:

  bool    isDark;
  Managed  *next;

  Managed() : isDark(false), next(nullptr) {}

  // a helper for freeing owned resources.
  virtual void freeChildren() {}

private:
  // object allocations should be prevented.
  void * operator new   (size_t) = delete;
  void * operator new[] (size_t) = delete;
  void   operator delete   (void *) = delete;
  void   operator delete[] (void*) = delete;

  // avoid copy.
  Managed(const Managed&) = delete;
  Managed& operator=(const Managed &) = delete;
};

} // namespace loxy


#endif