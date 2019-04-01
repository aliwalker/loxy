/// common.h - defines some macros used across loxy.
#ifndef loxy_common_h
#define loxy_common_h

#include <stddef.h>
#include <stdint.h>

#define UINT8_COUNT (UINT8_MAX + 1)
#define HEAP_GROW_PERCENT   0.5
#define INITIAL_HEAP_SIZE   1024
#define MAX_TEMP_ROOTS  5

// typedef unsigned int  size_t;

#define DEBUG
#ifdef DEBUG
  #define DEBUG_PRINT_CODE
  #define DEBUG_TRACE_EXECUTION
  #define DEBUG_TRACE_GC

  #include <stdio.h>
  #define ASSERT(condition, message) \
      do \
      { \
        if (!(condition)) \
        { \
          fprintf(stderr, "[%s:%d] Assert failed in %s(): %s\n", \
              __FILE__, __LINE__, __func__, message); \
          abort(); \
        } \
      } \
      while(0)

  #define UNREACHABLE() \
      do \
      { \
        fprintf(stderr, "[%s:%d] This code should not be reached in %s()\n", \
            __FILE__, __LINE__, __func__); \
        abort(); \
      } \
      while (0)
  
#endif
#endif