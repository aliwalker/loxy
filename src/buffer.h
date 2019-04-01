#ifndef loxy_buffer_h
#define loxy_buffer_h

#include <memory>

#include "common.h"
#include "memory.h"

namespace loxy {

class LoxyVM;

typedef unsigned int  size_t;

/// Buffer - A simple buffer type for holding different types of values.
template<typename T>
class Buffer {
  LoxyVM  &vm;
  ReallocateFn  reallocate;
public:
  size_t capacity;
  size_t count;
  T *data;

  Buffer(LoxyVM &vm, size_t initialCap = 8, ReallocateFn fn = LoxyReallocate);
  void append(T value);
};

template<typename T>
Buffer<T>::Buffer(LoxyVM &vm, size_t initialCap,
                  ReallocateFn fn): vm(vm), reallocate(fn),
                    capacity(initialCap), count(0), data(NULL) {}

template<typename T>
void Buffer<T>::append(T value) {
  // Grow if neccessary.
  if (this->capacity < this->count + 1) {
    size_t oldCap = this->capacity;
    this->capacity = grow_capacity(oldCap);
    this->data = (T *)reallocate(vm, this->data, oldCap, this->capacity);
  }
  this->data[this->count++] = value;
}

/// makeBuffer - creates a buffer of type [T] and returns a shared_ptr to it.
template<typename T>
std::shared_ptr<Buffer<T>> makeBuffer(LoxyVM &vm, ReallocateFn fn = LoxyReallocate) {
  return std::make_shared(new Buffer<T>(vm, 8, fn));
}

};
#endif