#include "chunk.h"

namespace loxy {

Chunk::~Chunk() {
  LoxyReallocate(vm, code, sizeof(uint8_t) * capacity, 0);
  LoxyReallocate(vm, lines, sizeof(int) * capacity, 0);
}

std::shared_ptr<Chunk> Chunk::makeChunk(LoxyVM &vm) {
  auto chunk = std::make_shared<Chunk>(vm);

  return chunk;
}

void Chunk::write(uint8_t byte, int line) {
  // Grow if necessary.
  if (capacity < count + 1) {
    size_t oldCap = capacity;
    capacity = grow_capacity(oldCap);
    code = static_cast<uint8_t*>(LoxyReallocate(vm, code,
                                                sizeof(uint8_t) * oldCap,
                                                sizeof(uint8_t) * capacity));
    lines = static_cast<int*>(LoxyReallocate(vm, lines,
                                             sizeof(int) * oldCap,
                                             sizeof(int) * capacity));
  }

  code[count] = byte;
  lines[count] = line;
  count++;
}

int Chunk::addConstant(Value value) {
  constants.append(value);
  return constants.count - 1;
}

}