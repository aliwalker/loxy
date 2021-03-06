#ifndef loxy_parser_h
#define loxy_parser_h

#include <functional>
#include "Scanner.h"
#include "VM/OpCode.h"
#include "VM/Chunk.h"

namespace loxy {

enum class Tok;
struct Token;
class Scanner;
class Value;
class Chunk;
class VM;

enum class OpCode: uint8_t;

// class Parser - the compiler for emitting bytecode.
class Parser {
private:

  Scanner *scanner_;
  VM      &vm;

  Token current;
  Token previous;
  bool hadError;
  bool panicMode;

  Chunk *currentChunk_;

  class FunctionScope;
  FunctionScope *currentFunc_;

public:

  Parser(VM &vm);

  // TODO: return [Function].
  /// parse - main interface for parsing [source] code and emitting
  ///   corresponding bytecode to [compilingChunk].
  bool parse(Chunk *compilingChunk, const char *source);

private:
  // data types for compiling.

  //typedef std::function<void(bool)> ParseFn;

  typedef void (Parser::*ParseFn)(bool);

  // Rules for expression pratt-parsing.
  struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    int precedence;
  };

  // struct Variable - represents local variables accessible by function scope.
  //  top-level variable is not represented by this struct.
  struct Variable {
    // The name token that contains source info.
    Token name;

    // The depth of the variable. Top-level variable has depth of 0.
    // -1 means the name is being declared yet usable.
    int   depth = -1;
  };

  // class FunctionScope - one scope stack per function.
  class FunctionScope {
  public:
    // local variables stacked by scope
    // TODO: maybe more local variables?
    Variable vars[UINT8_MAX];
    
    // count of [vars]
    int count;

    // current depth.
    int depth;

    // enclosing function.
    FunctionScope *enclosing;

    // TODO:
    // current function being compiled.
    // Function *function;
  public:

    FunctionScope(FunctionScope *enclosing = nullptr, int depth = 0)
      : count(0), depth(depth),
        enclosing(enclosing) {}

    // marks a local variable comes into scope yet available.
    int createLocal(Token name) {
      vars[count].name = name;
      vars[count].depth = depth;
      count++;
      return count - 1;
    }

    // marks var[index] as initialized.
    // must be called when initializer is emitted.
    void initLocal(uint8_t index) {
      assert(index < count && "Initializing variable out of scope!");
      vars[index].depth = depth;
    }

    // reads a local variable at [index].
    const Variable &getLocal(uint8_t index) const {
      assert(index < count && "Getting variable out of scope!");
      assert(vars[index].depth != -1 && "Getting variable not initialized yet!");
      return vars[index];
    }

    // pops [n] local variables.
    void pop(size_t n) {
      assert(count > n && "Popping too many variables!");
      count -= n;

      // reset depth.
      depth = vars[count - 1].depth;
    }
  };  // class ScopeInfo

private:

  // driver table for pratt parsing.
  static ParseRule rules[static_cast<int>(Tok::TOKEN_NUMS)];

  // adds [value] as constant to current compiling chunk.
  uint8_t makeConstant(Value value);

  Chunk &currentChunk() const {
    assert(currentChunk_ != nullptr && "Current chunk must not be nullptr");
    return *currentChunk_;
  }

  // beginScope - called when parser enters a new lexical scope, in current parsing function.
  void beginScope() { currentFunc_->depth++; }

  // endScope - called when parser exists current lexical scope.
  void endScope() {
    FunctionScope *current = currentFunc_;
    int varCount = 0;
    int depth = current->depth;

    while (current->count > 0 && current->getLocal(varCount - 1).depth >= depth) {
      varCount++;
    }
    
    current->pop(varCount);
  }

  void beginFunction(FunctionScope *function) {
    function->enclosing = currentFunc_;
    function->count = 0;

    if (currentFunc_ != nullptr) {
      function->depth = currentFunc_->depth;
    } else {
      function->depth = 0;
    }

    // TODO: add function's type.
  }

  void endFunction() {
    emitReturn();
    currentFunc_ = currentFunc_->enclosing;

    // TODO: return the function Object.
  }

private:
  // methods have to do with variables.
  //
  /// declareVariable - marks a variable as declared yet available for use.
  ///   returns an index of the variable in appropriate table.
  uint8_t declareVariable(const char *msg);

  /// declareLocal - declares a local variable.
  uint8_t declareLocal();

  /// declareGlobal - declares a global variable.
  uint8_t declareGlobal();

  /// defineVariable - marks a declared variable as available.
  void defineVariable(uint8_t var);

  /// resolveLocal - called when parsing a variable expression.
  //    returns -1 if not found. This method guarantees you don't use
  //    uninitialized local variable.
  int resolveLocal(const Token &name);

  /// identifierConstant - stores [name] which is an identifier, as a constant to
  ///   [currentChunk]'s constant table.
  uint8_t identifierConstant(Token name);

  /// identifiersEqual - compares the chars contained in [a] & [b].
  bool identifiersEqual(const Token &a, const Token &b);

private:
    // Top-down parsers for statements.
  //
  /// decalaration := varDeclaration
  void declaration();

  /// varDeclaration := "var" identifier ("=" expression)?
  void varDeclaration();
  void statement();
  void whileStatement();
  void forStatement();
  void ifStatement();
  void block();
  void expressionStatement();
  void printStatement();

  /// parsePrecedence - parses from current states, until parser
  ///   hits an expression lower than given [prec]. This is the core
  ///   of pratt-parsing.
  void parsePrecedence(int prec);

  // sub parsers for expressions.
  void expression();
  void and_(bool _);  // a == 5 and b == 2
  void or_(bool _);   // a == 5 or a == 6
  void binary(bool _);
  void atom(bool _);
  void number(bool _);
  void string(bool _);
  void variable(bool assignable);
  void unary(bool _);
  void grouping(bool _);

  // helpers for updating internal parser states.
  //
  //void initParser(Chunk *chunk);
  void advance();
  // compelling helper for matching current token with [type].
  void consume(Tok type, const char *msg);
  // non-compelling helper for matching current token with [type].
  bool match(Tok type);
  bool check(Tok type);

  // emitters that emits bytecode to given chunk.
  //
  void emit(uint8_t byte);
  void emit(OpCode op);
  void emitReturn();
  void emitConstant(Value value);

  /// emitJump - emits [jumpInst] and a 2-byte offset for jump. 
  ///   Returns the index of the offset in the compiling chunk.
  int emitJump(OpCode jumpInst);

  /// patchJump - replaces the jump instruction's arg(resides in [offset]) 
  ///   with the number bytes to skip to current end of bytecode.
  void patchJump(int index);

  void emitLoop(int loopStart);

  // error handling.
  //
  void errorAt(const Token &token, const char *msg);
  void errorAtCurrent(const char *msg);
  void error(const char *msg);

  /// synchronize - synchronizes current parsing error by discarding some
  ///   tokens.
  void synchronize();
}; // class Parser

} // namespace loxy

#endif