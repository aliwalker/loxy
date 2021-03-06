#ifndef loxy_common_h
#define loxy_common_h

#include <stddef.h>
#include <stdint.h>
#include <cassert>

#define UINT8_COUNT         (UINT8_MAX + 1)
#define HEAP_GROW_PERCENT   0.5
#define INITIAL_HEAP_SIZE   1024
#define MAX_TEMP_ROOTS      5

#define STACK_MAX           256

#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINT_CODE
  #define DEBUG_TRACE_EXECUTION
  #define DEBUG_TRACE_GC

  #include <stdio.h>

  #define UNREACHABLE()                             \
    do {                                            \
      assert(false && "reached unreachable!");      \
    }                                               \
    while (0)
  
#endif
#endif