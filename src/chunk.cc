#include "chunk.h"

namespace loxy {

void Chunk::write(uint8_t byte, int line) {
  code.push_back(byte);
  lines.push_back(line);
}

int Chunk::addConstant(Value value) {
  constants.push_back(value);
  return constants.size() - 1;
}

}