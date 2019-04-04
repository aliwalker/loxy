#include "chunk.h"

namespace loxy {

void Chunk::write(uint8_t byte, int line) {
  code.push_back(byte);
  lines.push_back(line);
}

int Chunk::addConstant(Value value) {
  // if the constant is already in the pool, return it.
  auto index = 0;
  for (auto &val : constants) {
    if (val == value) return index;
    index++;
  }

  // new constant.
  constants.push_back(value);
  return constants.size() - 1;
}

Value Chunk::getConstant(size_t index) const { return constants[index]; }

} // namespace loxy.