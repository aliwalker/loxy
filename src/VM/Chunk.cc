#include <stdio.h>
#include "Data/SmallVector.h"
#include "Chunk.h"
#include "Value.h"
#include "VM.h"

namespace loxy {

Chunk *Chunk::create(VM &vm) {
  void *mem = vm.reallocate(nullptr, 0, sizeof(Chunk));
  assert(mem != nullptr && "Out of memory");

  auto code_ = SmallVector<uint8_t>::create(vm);
  auto lines_ = SmallVector<int>::create(vm);
  auto constants_ = SmallVector<Value>::create(vm);

  return ::new(mem) Chunk(vm, code_, lines_, constants_);
}

void Chunk::destroy(VM &vm, Chunk **chunk) {
  if (*chunk == nullptr)  return;

  // free owned resources.
  SmallVector<uint8_t>::destroy(vm, &((*chunk)->code_));
  SmallVector<int>::destroy(vm, &((*chunk)->lines_));
  SmallVector<Value>::destroy(vm, &((*chunk)->constants_));

  // free chunk itself
  vm.reallocate(*chunk, sizeof(Chunk), 0);
  *chunk = nullptr;
}

uint8_t Chunk::read(size_t offset) const { return code()[offset]; }

size_t Chunk::size() const noexcept { return code_->count(); }

void Chunk::write(uint8_t byte, int line) {
  code_->push(byte);
  lines_->push(byte);
}

void Chunk::clear() {
  code_->clear();
  lines_->clear();
  constants_->clear();
}

int Chunk::addConstant(Value value) {
  // check for existence.
  if (constants_->indexOf(value) != -1) return constants_->indexOf(value);

  constants_->push(value);
  return constants_->count() - 1;
}

Value Chunk::getConstant(size_t index) const {
  assert(index < constants_->count() && "Index is too large in constant pool");
  return constants()[index];
}

//----========= helpers for printing chunks ===========----//
//
static int constInst(const char *name, Chunk *chunk, int offset)
{
  uint8_t idx = chunk->code()[offset + 1];
  printf("%-16s %4d '", name, idx);
  printf("%s\n", chunk->getConstant(idx).cString());
  return offset + 2;
}

static int simpleInst(const char *name, int offset)
{
  printf("%s\n", name);
  return offset + 1;
}

static int byteInst(const char *name, Chunk *chunk, int offset)
{
  uint8_t slot = chunk->code()[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2;
}

static int jumpInst(const char *name, int sign, Chunk *chunk, int offset)
{
  uint16_t jump = (uint16_t)(chunk->code()[offset + 1] << 8);
  jump |= chunk->code()[offset + 2];
  printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

static int Inst(Chunk *chunk, int offset) {
  printf("%04d ", offset);
  if (offset > 0 && chunk->lines()[offset] == chunk->lines()[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines()[offset]);
  }

  OpCode instruction = (OpCode)chunk->code()[offset];

  switch (instruction)
  {
  case OpCode::CONSTANT:  return constInst("CONSTANT", chunk, offset);
  case OpCode::NIL:       return simpleInst("NIL", offset);
  case OpCode::TRUE:      return simpleInst("TRUE", offset);
  case OpCode::FALSE:     return simpleInst("FALSE", offset);
  case OpCode::POP:       return simpleInst("POP", offset);
  case OpCode::GET_LOCAL: return byteInst("GET_LOCAL", chunk, offset);
  case OpCode::SET_LOCAL: return byteInst("SET_LOCAL", chunk, offset);
  case OpCode::DEFINE_GLOBAL: return constInst("DEFINE_GLOBAL", chunk, offset);
  case OpCode::SET_GLOBAL:    return constInst("SET_GLOBAL", chunk, offset);
  case OpCode::GET_GLOBAL:    return constInst("GET_GLOBAL", chunk, offset);
  case OpCode::EQUAL:         return simpleInst("EQUAL", offset);
  case OpCode::LESS:          return simpleInst("LESS", offset);
  case OpCode::GREATER:       return simpleInst("GREATER", offset);
  case OpCode::ADD:           return simpleInst("ADD", offset);
  case OpCode::SUBTRACT:      return simpleInst("SUBTRACT", offset);
  case OpCode::MULTIPLY:      return simpleInst("MULTIPLY", offset);
  case OpCode::DIVIDE:        return simpleInst("DIVIDE", offset);
  case OpCode::NOT:           return simpleInst("NOT", offset);
  case OpCode::NEGATE:        return simpleInst("NEGATE", offset);
  case OpCode::PRINT:         return simpleInst("PRINT", offset);
  case OpCode::JUMP:          return jumpInst("JUMP", 1, chunk, offset);
  case OpCode::JUMP_IF_FALSE: return jumpInst("JUMP_IF_FALSE", 1, chunk, offset);
  case OpCode::LOOP:          return jumpInst("LOOP", -1, chunk, offset);
  case OpCode::RETURN:        return simpleInst("RETURN", offset);
  }
}

void DisassembleChunk(Chunk *chunk, const char *name)
{
  printf("==== %s ====\n", name);

  for (int offset = 0; offset < chunk->size();) {
    offset = Inst(chunk, offset);
  }
}

} // namespace loxy