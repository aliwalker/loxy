#include <stdlib.h>

#include "Module.h"
#include "VM.h"
#include "Value.h"
#include "Data/SmallVector.h"
#include "Data/HashMap.h"

namespace loxy {

VM::VM():
  allocatedBytes(0),
  nextGC(1024 * 1024),
  first(nullptr) {

  modules_ = SmallVector<Module*>::create(*this);
  stringPool = HashMap::create(*this);
}

VM::~VM() {
  SmallVector<Module*>::destroy(*this, &modules_);
  HashMap::destroy(*this, &stringPool);
}

void *VM::reallocate(void *prev, size_t oldSize, size_t newSize) {
  allocatedBytes += newSize - oldSize;

  // [collectGarbage] will remove this object from
  // allocated list.
  if (newSize == 0) {
    free(prev);
    return nullptr;
  }
  
  if (newSize > oldSize) {
  #ifdef DEBUG_GC
    collectGarbage();
  #endif
    if (allocatedBytes > nextGC) {
      collectGarbage();
    }
  }

  return realloc(prev, newSize);
}

String *VM::findString(const char *chars,
                       int length,
                       uint32_t hash) {
  Value str;
}

InterpretResult VM::run(Module *module) {
  const Chunk *code = module->getBody();
  Value stack[STACK_MAX];
  Value *stackTop = stack;
  int ip = 0;

//----=== helpers ===----//

#define validate_numbers(a, b)                            \
  if (!(a).isNumber() || !(b).isNumber()) {               \
    runtimeError("Both operands must be numbers");        \
    return InterpretResult::Runtime_Error;                \
  }

#define arithmetics(op)                                   \
  do {                                                    \
    Value b = pop(); Value a = pop();                     \
    validate_numbers(a, b);                               \
    double result = (double)a op (double)b;               \
    push(Value(result));                                  \
  } while (false)

#define read_byte()     code->read(ip++)
#define read_short()    (uint16_t)read_byte() << 8 | read_byte()
#define read_string()   (String*)read_constant()
#define read_constant() code->getConstant(read_byte())

#define push(value)     *stackTop++ = value
#define pop()           *stackTop--
#define peek(distance)  *(stackTop - 1 - distance)
  
#define isFalsey(v)     (v).isNil() || ((v).isBool() && !((bool)(v)))

//----================----//

  while (true) {
    OpCode instruction = (OpCode)read_byte();
    switch (instruction) {
    case OpCode::CONSTANT: {
      Value constant = read_constant();
      push(constant);
      break;
    }
    case OpCode::NIL:   push(Value::Nil); break;
    case OpCode::TRUE:  push(Value::True); break;
    case OpCode::FALSE: push(Value::False); break;
    case OpCode::POP:   pop(); break;

    case OpCode::DEFINE_GLOBAL: {
      String *name = read_string();
      Value value = pop();
      module->addVariable(name, value);
    }

    case OpCode::GET_GLOBAL: {
      String *name = read_string();
      Value value;
      if (!module->getVariable(name, &value)) {
        // TODO:
        // report runtime error
        return InterpretResult::Runtime_Error;
      }
      push(value);
      break;
    }

    case OpCode::SET_GLOBAL: {
      String *name = read_string();
      Value value = peek(0);
      if (!module->setVariable(name, value)) {
        // TODO:
        // report runtime error
        return InterpretResult::Runtime_Error;
      }
      break;
    }

    case OpCode::SET_LOCAL: {
      uint8_t slot = read_byte();
      stack[slot] = peek(0);
      break;
    }
    
    case OpCode::GET_LOCAL: {
      uint8_t slot = read_byte();
      push(stack[slot]);
      break;
    }

    case OpCode::EQUAL: {
      Value b = pop();
      Value a = pop();
      push(a == b ? Value::True : Value::False);
      break;
    }

    case OpCode::GREATER: {
      Value b = pop();
      Value a = pop();

      validate_numbers(a, b);
      push(a > b);
    }

    case OpCode::LESS: {
      Value b = pop();
      Value a = pop();

      validate_numbers(a, b);
      push(b > a);
    }

    case OpCode::ADD: {
      Value b = pop();
      Value a = pop();

      if (a.isString() && b.isString()) {
        // TODO
      } else if (a.isNumber() && b.isNumber()) {
        double sum = (double)a + (double)b;
        push(Value(sum));
      } else {
        // TODO:
        // report error
        return InterpretResult::Runtime_Error;
      }
      break;
    }
    case OpCode::SUBTRACT:  arithmetics(-); break;
    case OpCode::MULTIPLY:  arithmetics(*); break;
    case OpCode::DIVIDE:    arithmetics(/); break;

    case OpCode::NOT: {
      Value v = pop();
      push(isFalsey(v) ? Value::True : Value::False);
      break;
    }

    case OpCode::NEGATE: {
      Value v = pop();
      if (!v.isNumber()) {
        // TODO:
        // report error
        return InterpretResult::Runtime_Error;
      }
      push(Value(-(double)v));
      break;
    }

    case OpCode::PRINT: {
      Value v = pop();
      // TODO:
      // print value
    }

    case OpCode::JUMP: {
      int offset = read_short();
      ip += offset;
      break;
    }

    case OpCode::JUMP_IF_FALSE: {
      uint16_t offset = read_short();
      if (isFalsey(peek(0)))  ip += offset;
      break;
    }

    case OpCode::RETURN: {
      return InterpretResult::Ok;
    }

    default:  UNREACHABLE();
    }
  }

#undef validate_numbers
#undef arithmetics
#undef read_bytes
#undef read_short
#undef read_string
#undef read_constant
}

} // namespace loxy