#include "Data/SmallVector.h"
#include "Chunk.h"
#include "Value.h"
#include "VM.h"

namespace loxy {

Chunk *Chunk::create(VM &vm) {
  void *mem = vm.reallocate(nullptr, 0, sizeof(Chunk));
  assert(mem != nullptr && "Out of memory");

  auto code = SmallVector<uint8_t>::create(vm);
  auto lines = SmallVector<int>::create(vm);
  auto constants = SmallVector<Value>::create(vm);

  return ::new(mem) Chunk(vm, code, lines, constants);
}

void Chunk::destroy(VM &vm, Chunk **chunk) {
  if (*chunk == nullptr)  return;

  // free owned resources.
  SmallVector<uint8_t>::destroy(vm, &((*chunk)->code));
  SmallVector<int>::destroy(vm, &((*chunk)->lines));
  SmallVector<Value>::destroy(vm, &((*chunk)->constants));

  // free chunk itself
  vm.reallocate(*chunk, sizeof(Chunk), 0);
  *chunk = nullptr;
}

uint8_t Chunk::read(size_t offset) const { return (*code)[offset]; }

size_t Chunk::size() const noexcept { return code->count(); }

void Chunk::write(uint8_t byte, int line) {
  code->push(byte);
  lines->push(byte);
}

void Chunk::clear() {
  code->clear();
  lines->clear();
  constants->clear();
}

int Chunk::addConstant(Value value) {
  // check for existence.
  if (constants->indexOf(value) != -1) return constants->indexOf(value);

  constants->push(value);
  return constants->count() - 1;
}

Value Chunk::getConstant(size_t index) const {
  assert(index < constants->count() && "Index is too large in constant pool");
  return (*constants)[index];
}

} // namespace loxy