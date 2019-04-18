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

  Scanner &scanner;
  VM      &vm;

  Token current;
  Token previous;
  bool hadError;
  bool panicMode;

  Chunk *currentChunk_;

  class Scope;
  Scope *currentScope;

public:

  /// Parser - takes [source] as source code & initializes
  ///   internal states. Source can be nullptr for convenience.
  Parser(VM &vm, Scanner &scanner);

  /// parse - main interface for parsing [source] code and emitting
  ///   corresponding bytecode to [compilingChunk].
  bool parse(Chunk &compilingChunk, const char *source);

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

  /// struct Local - represents local variables.
  struct Local {
    // The name token that contains source info.
    Token name;

    // The depth of the variable. Top-level variable has depth of 0.
    // -1 means the name is being declared yet usable.
    int   depth = -1;
  };

  /// class ScopeInfo - bookkeeping info of lexical scopes used
  ///   by Parser.
  class ScopeInfo {
  private:
    // local variables stacked by scope
    Local vars[UINT8_MAX];
    
    // count of [vars]
    int count = 0;

    // current depth.
    int depth = 0;

  public:

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
    const Local &getLocal(uint8_t index) const {
      assert(index < count && "Getting variable out of scope!");
      assert(vars[index].depth != -1 && "Getting variable not initialized yet!");
      return vars[index];
    }

    // called when entered a new lexical scope.
    void pushScope() { depth++; }

    // called when current scope ends.
    // returns the number of locals that should be popped.
    int popScope() {
      assert(depth > 0 && "popScope called at top-level scope!");

      int oldCount = count;
      while (count > 0 && vars[count - 1].depth == depth) count--;

      depth--;
      return oldCount - count;
    }
  };  // class ScopeInfo

  // class Scope - a new scope is created when parser enters a new
  //  function body.
  class Scope {
  public:
    // enclosing function of current function.
    Scope *enclosing;

    ScopeInfo scopeInfo;

    // TODO:
    // Function *function;
  }; // class Scope

private:

  // driver table for pratt parsing.
  static ParseRule rules[static_cast<int>(Tok::TOKEN_NUMS)];

  // adds [value] as constant to current compiling chunk.
  uint8_t makeConstant(Value value);

  Chunk &currentChunk() const {
    assert(currentChunk_ != nullptr && "Current chunk must not be nullptr");
    return *currentChunk_;
  }

private:
  // methods have to do with variables.
  //
  /// parseDeclaration - parses a variable name. Called on varDeclaration. 
  ///   if the var being parsed is a global, stores it into
  ///   global symTable and returns the index of it. Otherwise(var being a local)
  ///   returns -1 and do nothing.
  uint8_t parseVariable(const char *msg);

  /// declareVariable - "declares" the current token as variable yet usable.
  void declareVariable();

  /// defineVariable - "defines" [var] as a variable. Vars are usable after "being defined".
  void defineVariable(uint8_t var);

  /// resolveLocal - resolves to a local variable if there is one. Returns the index of it
  ///   in [this->locals] if it exists; otherwise returns -1.
  int resolveLocal(Token &name);

  /// identifierConstant - stores [name] which is an identifier, as a constant to
  ///   [currentChunk]'s constant table.
  uint8_t identifierConstant(Token &name);

  /// identifiersEqual - returns true if [name1] & [name2] are considered equal.
  bool identifiersEqual(Token &name1, Token &name2);

  /// markInitialized - marks the current local variable as "ready for use".
  ///   This is called by [defineVariable].
  void markInitialized();

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
  void initParser(Chunk *chunk);
  void advance();
  void consume(Tok type, const char *msg);
  void endParser();
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
  void patchJump(int offset);

  void emitLoop(int loopStart) {
    emit(OpCode::LOOP);

    int offset = currentChunk().size() - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body too large");

    // high bits
    emit((offset >> 8) & 0xff);
    // low bits
    emit(offset & 0xff);
  }

  // error handling.
  //
  void errorAt(const Token &token, const char *msg);
  void errorAtCurrent(const char *msg);
  void error(const char *msg);

  /// synchronize - synchronizes current parsing error by discarding some
  ///   tokens.
  void synchronize();

  // scope handler.
  void beginScope();
  void endScope();
}; // class Parser

} // namespace loxy

#endif