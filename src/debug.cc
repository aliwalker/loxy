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
    printf("    | ");
  } else {
    printf("%4d ", chunk.lines[offset]);
  }

  OpCode instruction = (OpCode)chunk.code[offset];
  switch (instruction) {
  case OpCode::CONSTANT:
    return constantInstruction("CONSTANT", chunk, offset);
  case OpCode::NIL:
    return simpleInstruction("NIL", chunk, offset);
  case OpCode::TRUE:
    return simpleInstruction("TRUE", chunk, offset);
  case OpCode::FALSE:
    return simpleInstruction("FALSE", chunk, offset);
  case OpCode::POP:
    return simpleInstruction("POP", chunk, offset);
  case OpCode::RETURN:
    return simpleInstruction("RETURN", chunk, offset);
  case OpCode::PRINT:
    return simpleInstruction("PRINT", chunk, offset);
  }
}

size_t ChunkPrinter::constantInstruction(const char *inst, Chunk &chunk, size_t offset) {
  auto constantIdx = chunk.code[offset + 1];
  printf("%-16s %4d '", inst, constantIdx);
  auto constant = chunk.getConstant(constantIdx);
  constant.toString();
  printf("'\n");
  return offset + 2;
}

size_t ChunkPrinter::simpleInstruction(const char *inst, Chunk &chunk, size_t offset) {
  printf("%s\n", inst);
  return offset + 1;
}

}