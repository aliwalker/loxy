#include "Chunk.h"
#include "Value.h"

namespace loxy {

class Value;

void Chunk::write(uint8_t byte, int line) {
  code.push_back(byte);
  lines.push_back(byte);
}

void Chunk::clear() {
  code.clear();
  lines.clear();
}

int Chunk::addConstant(Value value) {
  // check for existence.
  for (auto i = constants.begin(); i != constants.end(); i++) {
    if (*i == value) {
      return i - constants.begin();
    }
  }

  constants.push_back(value);
  return constants.size() - 1;
}

Value Chunk::getConstant(size_t index) const {
  assert(index < constants.size() && "Index is too large in constant pool");
  return constants[index];
}

} // namespace loxy