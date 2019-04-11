#include <stdio.h>

#include "debug.h"
#include "chunk.h"

namespace loxy {

void ChunkPrinter::printChunk(Chunk &chunk, const char *name) {
  printf("== %s ==\n", name);
  for (size_t offset = 0; offset < chunk.size();) {
    offset = printInstruction(chunk, offset);
  }
}

size_t ChunkPrinter::printInstruction(Chunk &chunk, size_t offset) {
  printf("%04d ", (int)offset);

  if (offset > 0 && chunk.lines[offset] == chunk.lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk.lines[offset]);
  }

  OpCode instruction = (OpCode)chunk.code[offset];
  switch (instruction) {
  case OpCode::CONSTANT:
    return constantInstruction("CONSTANT", chunk, offset);
  case OpCode::NIL:
    return simpleInstruction("NIL", chunk, offset);
  case OpCode::NEGATE:
    return simpleInstruction("NEGATE", chunk, offset);
  case OpCode::TRUE:
    return simpleInstruction("TRUE", chunk, offset);
  case OpCode::FALSE:
    return simpleInstruction("FALSE", chunk, offset);
  case OpCode::POP:
    return simpleInstruction("POP", chunk, offset);
  case OpCode::GET_GLOBAL:
    return constantInstruction("GET_GLOBAL", chunk, offset);
  case OpCode::SET_GLOBAL:
    return constantInstruction("GET_GLOBAL", chunk, offset);
  case OpCode::DEFINE_GLOBAL:
    return constantInstruction("DEFINE_GLOBAL", chunk, offset);
  case OpCode::GET_LOCAL:
    return byteInstruction("GET_LOCAL", chunk, offset);
  case OpCode::SET_LOCAL:
    return byteInstruction("SET_LOCAL", chunk, offset);
  case OpCode::EQUAL:
    return simpleInstruction("EQUAL", chunk, offset);
  case OpCode::GREATER:
    return simpleInstruction("GREATER", chunk, offset);
  case OpCode::LESS:
    return simpleInstruction("LESS", chunk, offset);
  case OpCode::ADD:
    return simpleInstruction("ADD", chunk, offset);
  case OpCode::SUBTRACT:
    return simpleInstruction("SUBTRACT", chunk, offset);
  case OpCode::MULTIPLY:
    return simpleInstruction("MULTIPLY", chunk, offset);
  case OpCode::DIVIDE:
    return simpleInstruction("DIVIDE", chunk, offset);
  case OpCode::NOT:
    return simpleInstruction("NOT", chunk, offset);
  case OpCode::RETURN:
    return simpleInstruction("RETURN", chunk, offset);
  case OpCode::PRINT:
    return simpleInstruction("PRINT", chunk, offset);
  }
}

size_t ChunkPrinter::constantInstruction(const char *inst, Chunk &chunk, size_t offset) {
  auto constantIdx = chunk.code[offset + 1];
  auto constant = chunk.getConstant(constantIdx);

  printf("%-16s %4d '", inst, constantIdx);
  printf("%s'\n", constant.toString().c_str());
  return offset + 2;
}

size_t ChunkPrinter::simpleInstruction(const char *inst, Chunk &chunk, size_t offset) {
  printf("%s\n", inst);
  return offset + 1;
}

size_t ChunkPrinter::byteInstruction(const char *inst, Chunk &chunk, size_t offset) {
  uint8_t slot = chunk.code[offset + 1];
  printf("%-16s %4d\n", inst, slot);
  return offset + 2;
}

}