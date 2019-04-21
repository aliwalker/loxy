#include "Chunk.h"
#include "Value.h"

namespace loxy {

class Value;

void Chunk::write(uint8_t byte, int line) {
  code.push(byte);
  lines.push(byte);
}

void Chunk::clear() {
  code.clear();
  lines.clear();
}

int Chunk::addConstant(Value value) {
  // check for existence.
  if (constants.indexOf(value) != -1) return constants.indexOf(value);

  constants.push(value);
  return constants.count() - 1;
}

Value Chunk::getConstant(size_t index) const {
  assert(index < constants.count() && "Index is too large in constant pool");
  return constants[index];
}

} // namespace loxy