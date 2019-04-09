#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "common.h"
#include "compiler.h"
#include "value.h"
#include "vm.h"
#ifdef DEBUG
  #include "debug.h"
#endif

namespace loxy {

uint8_t LoxyVM::readByte() {
  auto chunk = currModule->getChunk();
  return chunk->read(offset++);
}

Value LoxyVM::readConstant() {
  auto chunk = currModule->getChunk();
  auto constIndx = readByte();
  return chunk->getConstant(constIndx);
}

StringRef LoxyVM::readString() {
  auto value = (ObjectRef)readConstant();
  return static_cast<String*>(value);
}

void LoxyVM::push(Value value) {
  stack.push_back(value);
}

Value LoxyVM::peek(int distance) {
  return stack[stack.size() - 1 - distance];
}

Value LoxyVM::pop() {
  Value top = stack.back();
  stack.pop_back();
  return top;
}

InterpretResult LoxyVM::interpret(const char *source, const char *module) {
  auto mod = Module::create(*this, module);

  if (!(Compiler::compileModule(*this, source, *mod))) {
    currModule = static_cast<Module*>(currModule->next);
    return InterpretResult::Compile_Error;
  }
  
  mod->next = currModule;
  currModule = mod;

  return run();
}

static bool isFalsy(Value value) {
  return value.isNil() || (value.isBool() && !(bool)value);
}

InterpretResult LoxyVM::run() {
  auto chunk = currModule->getChunk();
#ifdef DEBUG
  ChunkPrinter::printChunk(*chunk, "main");
#endif

#define BIN_OP(op)  \
  do { \
    if (!peek(0).isNumber() || !peek(1).isNumber()) { \
      runtimeError("Operands must be numbers.");  \
      return InterpretResult::Runtime_Error;  \
    } \
    double b = (double)pop();  \
    double a = (double)pop();  \
    push(Value( a op b ));  \
  } while (false)

  while (true) {
    OpCode instruction = (OpCode)readByte();
    switch (instruction) {
    case OpCode::CONSTANT: {
      Value constant = readConstant();
      push(constant);
      break;
    }
    case OpCode::NIL:     push(Value::Nil); break;
    case OpCode::TRUE:    push(Value(true)); break;
    case OpCode::FALSE:   push(Value(false)); break;
    case OpCode::POP:     pop(); break;

    case OpCode::GET_LOCAL: {
      uint8_t slot = readByte();
      push(stack[slot]);
      break;
    }

    case OpCode::SET_LOCAL: {
      uint8_t slot = readByte();
      stack[slot] = peek(0);
    }

    case OpCode::GET_GLOBAL: {
      StringRef name = readString();
      Value value{false};
      if (currModule->getGlobal(name, &value)) {
        runtimeError("Undefined variable '%s'.", (char*)name);
        return InterpretResult::Runtime_Error;
      }
      push(value);
      break;
    }

    case OpCode::SET_GLOBAL: {
      StringRef name = readString();
      Value value = peek(0);
      if (!currModule->setGlobal(name, value)) {
        runtimeError("Undefined variable '%s'.", (char*)name);
        return InterpretResult::Runtime_Error;
      }
    }

    case OpCode::DEFINE_GLOBAL: {
      StringRef name = readString();
      Value value = peek(0);
      currModule->setGlobal(name, value);
      pop();
      break;
    }

    case OpCode::EQUAL: {
      Value b = pop();
      Value a = pop();

      push(a == b ? Value::True : Value::False);
      break;
    }

    case OpCode::GREATER: BIN_OP(>); break;
    case OpCode::LESS: BIN_OP(<); break;
    case OpCode::ADD: {
      auto a = peek(0);
      auto b = peek(1);

      // if (a.isObj() && b.isObj()) {
      //   auto aObject = (LoxyObj*)a;
      //   auto bObject = (LoxyObj*)b;
      //   if (dynamic_cast<LoxyStringRef>(aObject) != nullptr && 
      //       dynamic_cast<LoxyStringRef>(bObject) != nullptr) {
          
      //   }
      // }
      break;
    }

    case OpCode::SUBTRACT: BIN_OP(-); break;
    case OpCode::MULTIPLY: BIN_OP(*); break;
    case OpCode::DIVIDE: BIN_OP(/); break;
    case OpCode::NOT: {
      Value v = pop();
      push(Value(isFalsy(v) ? Value::True : Value::False));
      break;
    }
    case OpCode::NEGATE: {
      Value v = pop();
      if (!v.isNumber()) {
        runtimeError("Operand must be a number");
        return InterpretResult::Runtime_Error;
      }
      push(Value(-(double)v));
      break;
    }
    case OpCode::PRINT: {
      Value v = pop();

      printf("%s", v.toString().c_str());
      break;
    }
    case OpCode::RETURN:  return InterpretResult::Ok;

    default:  UNREACHABLE();
    }
  }
}

void *LoxyVM::newObject(size_t size) {
  ObjectRef obj = reinterpret_cast<ObjectRef>(realloc(nullptr, size));

  allocatedBytes += size;
  obj->next = first;
  first = obj;

  return reinterpret_cast<void*>(obj);
}

void LoxyVM::runtimeError(const char *format, ...) {

}

} // namespace loxy