#ifndef loxy_small_vector_h
#define loxy_small_vector_h

#include "Common.h"
#include "Managed.h"
#include "VM/VM.h"

namespace loxy {

// class SmallVector - a vector class used by Loxy's VM.
//  T must be primitive types or pointer types.
template<typename T>
class SmallVector : public Managed {
  friend class VM;

public:
  static const size_t MIN_CAP = 8;
  static const size_t GROW_PERCENT = 2;

private:
  VM &vm;
  int count_;
  int capacity_;

  // underlying blob of memory
  T *buffer_;

  explicit SmallVector(VM &vm, size_t cap = MIN_CAP)
  : vm(vm),
    count_(0),
    capacity_(0),
    buffer_(nullptr) {
    ensureCapacity(cap < MIN_CAP ? MIN_CAP : cap);
  }

public:

  // creates a VM managed SmallVector instance.
  static SmallVector<T> *create(VM &vm, size_t cap = MIN_CAP) {
    void *mem = vm.reallocate(nullptr, 0, sizeof(SmallVector<T>));

    assert(mem != nullptr && "Out of memory");
    return ::new(mem) SmallVector<T>(vm, cap);
  }

  static void destroy(VM &vm, SmallVector<T> **vector) {
    if (*vector == nullptr) return;

    // free buffer_
    vm.reallocate((*vector)->buffer_, (*vector)->capacity_ * sizeof(T), 0);
    (*vector)->count_ = 0;
    (*vector)->capacity_ = 0;
    (*vector)->buffer_ = nullptr;

    // free the vector itself
    vm.reallocate(*vector, sizeof(SmallVector<T>), 0);
    (*vector) = nullptr;
  }

  void freeChildren() {
    vm.reallocate(buffer_, capacity_ * sizeof(T), 0);
  }

  int count() const { return count_; }
  int capacity() const { return capacity_; }
  bool isEmpty() const { return count_ == 0; }

  void push(T elem) {
    ensureCapacity(count_ + 1);
    buffer_[count_++] = elem;
  }

  void push(const SmallVector<T> &another) {
    ensureCapacity(count_ + another.count_);
    for (int i = 0; i < another.count_; i++) buffer_[count_++] = another[i];
  }

  T pop() {
    assert(count_ > 0 && "Popping an empty vector!");
    T elem = buffer_[count_ - 1];
    count_--;
    return elem;
  }

  T& operator[] (int index) {
    assert(count_ > index && "Index of out range!");
    return buffer_[index];
  }

  const T& operator[] (int index) const {
    assert(count_ > index && "Index of out range!");
    return buffer_[index];
  }

  T back() {
    assert(count_ > 0 && "Reading an empty vector!");
    return buffer_[count_ - 1];
  }

  void reset() {
    count_ = 0;
  }

  void clear() {
    vm.reallocate(buffer_, capacity_ * sizeof(T), 0);
    count_ = 0;
    capacity_ = 0;
  }

  int indexOf(const T& elem) const {
    for (int i = count_ - 1; i >= 0; i--) {
      if (elem == buffer_[i]) return i;
    }
    return -1;
  }

private:

  inline void ensureCapacity(size_t size) {
    // enough memory for now.
    if (capacity_ > size) return;

    // grow
    int newCap = capacity_ == 0 ? MIN_CAP : capacity_ * GROW_PERCENT;
    while (newCap < size) newCap *= GROW_PERCENT;
    
    // update
    buffer_ = (T*)vm.reallocate(buffer_, capacity_ * sizeof(T), newCap * sizeof(T));
    capacity_ = newCap;
  }
};

} // namespace loxy


#endif