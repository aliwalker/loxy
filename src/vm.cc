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
    case OpCode::TRUE:    push(Value::True); break;
    case OpCode::FALSE:   push(Value::False); break;
    case OpCode::POP:     pop(); break;

    case OpCode::GET_LOCAL: {
      uint8_t slot = readByte();
      push(stack[slot]);
      break;
    }

    case OpCode::SET_LOCAL: {
      uint8_t slot = readByte();
      stack[slot] = peek(0);
      break;
    }

    case OpCode::GET_GLOBAL: {
      StringRef name = readString();
      Value value(false);
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
      // "set" is simply an expression which evals to [value].
      break;
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
      auto b = pop();
      auto a = pop();

      if (a.isString() && b.isString()) {
        String *aString = (String*)a;
        String *bString = (String*)b;
        String *result = aString->concat(*this, bString);
        push(Value(result));
      } else if (a.isNumber() && b.isNumber()) {
        double aNum = (double)a;
        double bNum = (double)b;
        push(Value(aNum + bNum));
      } else {
        runtimeError("Operands must be two numbers or two strings.");
        return InterpretResult::Runtime_Error;
      }
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

      printf("%s\n", v.toString().c_str());
      break;
    }
    case OpCode::RETURN: {
      offset = 0;
      return InterpretResult::Ok;
    }

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