#include <cassert>
#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include "common.h"
#include "compiler.h"
#include "value.h"

namespace loxy {

///----==================================================----///
///                       Scanner                            ///
///----==================================================----///
/// Tok - types of tokens.
enum class Tok {
  // Single-character tokens.     
  LEFT_PAREN, RIGHT_PAREN,
  LEFT_BRACE, RIGHT_BRACE,
  COMMA, DOT, MINUS, PLUS,
  SEMICOLON, SLASH, STAR,

  // One or two character tokens.           
  BANG, BANG_EQUAL,
  EQUAL, EQUAL_EQUAL,
  GREATER, GREATER_EQUAL,
  LESS, LESS_EQUAL,

  // Literals.
  IDENTIFIER, STRING, NUMBER,

  // Keywords.
  AND, CLASS, ELSE, FALSE,
  FOR, FUN, IF, NIL, OR,
  PRINT, RETURN, SUPER, THIS,
  TRUE, VAR, WHILE, 

  ERROR,
  _EOF
};

/// struct Token - a struct that holds enought info for a token.
struct Token {
  Tok         type;
  const char  *start;

  int         length;
  int         line;
};

struct Scanner {
  // The start of the current token.
  const char  *start;
  
  // The position of current scanning.
  const char  *current;

  // Line number in the source code.
  int line;

  // A flag indicates whether scanner was initialized.
  bool  initialized;
private:
  // Helpers for keeping internal states.
  bool isAtEnd();
  char advance();
  char peek();
  char peekNext();
  bool match(char expected);

  // Makes token from current states.
  Token makeToken(Tok type);
  Token errorToken(const char *msg);

  void skipWhitespace();
  Tok checkKeyword(size_t start, size_t len, const char *rest, Tok type);
  Tok identifierType();

  // Sub scanners that scan tokens of literal type.
  Token identifier();
  Token number();
  Token string();
public:
  Scanner(const char *source = nullptr) {
    initialized = source == nullptr ? false : true;
    if (!initialized) return;
    init(source);
  }

  /// scanToken - scans a token on demand.
  Token scanToken();
public:

  bool isInitialized() const { return initialized; }

  /// init - helper for initializing [scanner].
  void init(const char *source);
};  // Scanner

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

void Scanner::init(const char *source) {
  assert(source != nullptr && "source code for scanner initialization must not be nullptr");
  start = source;
  current = source;
  line = 1;
}

bool Scanner::isAtEnd() {
  return *current == '\0';
}

char Scanner::advance() {
  current++;
  return current[-1];
}

char Scanner::peek() {
  return *current;
}

char Scanner::peekNext() {
  if (isAtEnd())  return '\0';
  return current[1];
}

bool Scanner::match(char expected) {
  if (isAtEnd())  return false;
  if (*current != expected) return false;
  current++;
  return true;
}

Token Scanner::makeToken(Tok type) {
  Token token;
  token.type = type;
  token.start = start;
  token.length = (int)(current - start);
  token.line = line;
  return token;
}

Token Scanner::errorToken(const char *msg) {
  Token token;
  token.type = Tok::ERROR;
  token.start = msg;
  token.length = (int)strlen(msg);
  token.line = line;

  return token;
}

void Scanner::skipWhitespace() {
  while (true) {
    auto c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
      
    case '\n':
      line++;
      advance();
      break;
      
    case '/':
      if (peekNext() == '/') {
        // A comment goes until the end of the line.
        while (peek() != '\n' && !isAtEnd()) advance();
      } else {
        return;
      }
      break;
      
    default:
      return;
    }
  }
}

Tok Scanner::checkKeyword(size_t pos, size_t len, const char *rest, Tok type) {
  if (current - start == pos + len && memcmp(start + pos, rest, len) == 0) {
    return type;
  }
  return Tok::IDENTIFIER;
}

Tok Scanner::identifierType() {
  switch (*start) {
  case 'a': return checkKeyword(1, 2, "nd", Tok::AND);
  case 'c': return checkKeyword(1, 4, "lass", Tok::CLASS);
  case 'e': return checkKeyword(1, 3, "lse", Tok::ELSE);
  case 'f':
    if (current - start > 1) {
      switch (start[1]) {
      case 'a': return checkKeyword(2, 3, "lse", Tok::FALSE);
      case 'o': return checkKeyword(2, 1, "r", Tok::FOR);
      case 'u': return checkKeyword(2, 1, "n", Tok::FUN);
      }
    }
    break;
  case 'i': return checkKeyword(1, 1, "f", Tok::IF);
  case 'n': return checkKeyword(1, 2, "il", Tok::NIL);
  case 'o': return checkKeyword(1, 1, "r", Tok::OR);
  case 'p': return checkKeyword(1, 4, "rint", Tok::PRINT);
  case 'r': return checkKeyword(1, 5, "eturn", Tok::RETURN);
  case 's': return checkKeyword(1, 4, "uper", Tok::SUPER);
  case 't':
    if (current - start > 1) {
      switch (start[1]) {
      case 'h': return checkKeyword(2, 2, "is", Tok::THIS);
      case 'r': return checkKeyword(2, 2, "ue", Tok::TRUE);
      }
    }
    break;
  case 'v': return checkKeyword(1, 2, "ar", Tok::VAR);
  case 'w': return checkKeyword(1, 4, "hile", Tok::WHILE);
  }

  return Tok::IDENTIFIER;
}

Token Scanner::identifier() {
  while (isAlpha(peek()) || isDigit(peek())) advance();
  
  return makeToken(identifierType());
}

Token Scanner::number() {
  while (isDigit(peek())) advance();
  
  // fractional.
  if (peek() == '.' && isDigit(peekNext())) {
    // Consume the "."
    advance();
    
    while (isDigit(peek())) advance();
  }
  
  return makeToken(Tok::NUMBER);
}

Token Scanner::string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') line++;
    advance();
  }
  
  if (isAtEnd()) return errorToken("Unterminated string.");
  
  // closing "
  advance();
  return makeToken(Tok::STRING);
}

Token Scanner::scanToken() {
  if (initialized == false) {
    return errorToken("scanner has not been initialized yet!");
  }
  skipWhitespace();
  
  start = current;
    
  if (isAtEnd()) return makeToken(Tok::_EOF);
  
  auto c = advance();
  
  if (isDigit(c)) return number();
  if (isAlpha(c)) return identifier();
  
  switch (c) {
  case '(': return makeToken(Tok::LEFT_PAREN);
  case ')': return makeToken(Tok::RIGHT_PAREN);
  case '{': return makeToken(Tok::LEFT_BRACE);
  case '}': return makeToken(Tok::RIGHT_BRACE);
  case ';': return makeToken(Tok::SEMICOLON);
  case ',': return makeToken(Tok::COMMA);
  case '.': return makeToken(Tok::DOT);
  case '-': return makeToken(Tok::MINUS);
  case '+': return makeToken(Tok::PLUS);
  case '/': return makeToken(Tok::SLASH);
  case '*': return makeToken(Tok::STAR);
  case '!':
      return makeToken(match('=') ? Tok::BANG_EQUAL : Tok::BANG);
  case '=':
      return makeToken(match('=') ? Tok::EQUAL_EQUAL : Tok::EQUAL);
  case '<':
      return makeToken(match('=') ? Tok::LESS_EQUAL : Tok::LESS);
  case '>':
      return makeToken(match('=') ?
                        Tok::GREATER_EQUAL : Tok::GREATER);
      
  case '"': return string();
  }
  
  return errorToken("Unexpected character.");
} // scanToken

///----==================================================----///
///                       Parser                             ///
///----==================================================----///

enum class Precedence {
  NONE,
  ASSIGNMENT,  // =
  OR,          // or
  AND,         // and
  EQUALITY,    // == !=
  COMPARISON,  // < > <= >=
  TERM,        // + -
  FACTOR,      // * /
  UNARY,       // ! - +
  CALL,        // . () []
  PRIMARY
};

typedef std::function<void(bool)> ParseFn;

/// Rules for expression pratt-parsing.
struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

/// struct Local - represents local variables.
struct Local {
  // The name token that contains source info.
  Token name;

  // The depth of the variable. Top-level variable has depth of 0.
  // -1 means the name is being declared yet usable.
  int   depth = -1;
};

/// struct Locals - a structrue for manipulating current local variables 
///   in scope.
struct Locals {
  Local vars[UINT8_COUNT];

  // number of variables in [vars].
  int count = 0;
public:
  Locals() {}

  /// addLocals - adds a local variable to underlying [vars].
  void add(Token name) {
    if (count == UINT8_COUNT) {
      // error("Too many local variables in function.");
      return;
    }
    Local &var = vars[count++];
    var.name = name;
  }

  Local &get(uint8_t index) {
    return vars[index];
  }

  Local &back() {
    return vars[count - 1];
  }

  void pop() {
    count--;
  }

  void set(int index, int depth) {
    Local &var = vars[index];
    var.depth = depth;
  }
};

/// class Parser - Parser class for Loxy. Parse is the actual compiler for Loxy, since it
///   lexes, parses and emits for current function scope.
class Parser {
  LoxyVM &vm;

  // from which Parser pulls token out.
  Scanner &scanner;
  Token current{};
  Token previous{};
  bool hadErorr = false;
  bool panicMode = false;

  // a closure returning the chunk for current compilation.
  Chunk *_currentChunk = nullptr;

  Locals locals{};

  // The current level of block scope nesting.
  size_t scopeDepth = 0;
private:
  /// getRule - extracts rule from [rules] that matches [type].
  ParseRule &getRule(Tok type);

  /// makeConstant - adds [value] as constant to current compiling chunk.
  uint8_t makeConstant(Value value);

  Chunk &currentChunk() {
    assert(_currentChunk != nullptr && "Current chunk must not be nullptr");
    return *_currentChunk;
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
public:
  /// Parser - takes [source] as source code & initializes
  ///   internal states. Source can be nullptr for convenience.
  Parser(LoxyVM &vm, Scanner &scanner);

  /// parse - main interface for parsing [source] code and emitting
  ///   corresponding bytecode to [compilingChunk].
  bool parse(Chunk &compilingChunk, const char *source);

private:
  // Top-down parsers for statements.
  //
  /// decalaration := varDeclaration
  void declaration();

  /// varDeclaration := "var" identifier ("=" expression)?
  void varDeclaration();
  void statement();
  void forStatement();
  void ifStatement();
  void block();
  void expressionStatement();
  void printStatement();

  /// parsePrecedence - parses from current states, until parser
  ///   hits an expression lower than given [prec]. This is the core
  ///   of pratt-parsing.
  void parsePrecedence(Precedence prec);

  // sub parsers for expressions.
  void expression();
  void and_();  // a == 5 and b == 2
  void or_();   // a == 5 or a == 6
  void binary();
  void literal();
  void number();
  void string();
  void variable(bool canAssign);
  void unary();
  void grouping();

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
};  // class Parser

// parsers
//
Parser::Parser(LoxyVM &vm, Scanner &scanner): vm(vm), scanner(scanner) {
  hadErorr = false;
  panicMode = false;
}

bool Parser::parse(Chunk &compilingChunk, const char *source) {
  assert(source != nullptr && "source must not be null");
  scanner.init(source);

  initParser(&compilingChunk);
  advance();

  while (!match(Tok::_EOF)) {
    declaration();
  }

  endParser();
  return !hadErorr;
}

void Parser::declaration() {
  if (match(Tok::VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  if (panicMode) synchronize(); 
}

void Parser::varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  // initialization.
  if (match(Tok::EQUAL)) {
    expression();
  } else {
    // implicit nil assignment.
    emit((uint8_t)OpCode::NIL);
  }
  match(Tok::SEMICOLON);

  defineVariable(global);
}

uint8_t Parser::parseVariable(const char *errorMsg) {
  consume(Tok::IDENTIFIER, errorMsg);

  // soon as we've consumed variable name, mark it as declared.
  declareVariable();

  // if this is a local, exist the function.
  if (scopeDepth > 0) return 0;

  // store it into identifier table.
  return identifierConstant(previous);
}

void Parser::statement() {
  if (match(Tok::FOR)) {
    forStatement();
  } else if (match(Tok::IF)) {
    ifStatement();
  } else if (match(Tok::PRINT)) {
    printStatement();
  } else if (match(Tok::LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

void Parser::forStatement() {
  // scope for loop var
  beginScope();

  consume(Tok::LEFT_PAREN, "Expect '(' after 'for'");

  // initializer
  if (match(Tok::VAR)) {
    varDeclaration();
  } else if (match(Tok::SEMICOLON)) {

  } else {
    expressionStatement();
  }

  // the position of condition.
  int loopStart = currentChunk().size();
  int exitJump = -1;

  // condition
  if (!match(Tok::SEMICOLON)) {
    expression();
    consume(Tok::SEMICOLON, "Expect ';' after condition");
    exitJump = emitJump(OpCode::JUMP_IF_FALSE);

    // cond still true
    emit(OpCode::POP);
  }

  // increment
  if (!match(Tok::RIGHT_PAREN)) {
    int bodyJump = emitJump(OpCode::JUMP);

    // the index of the increment byte
    int incrementStart = currentChunk().size();
    
    // increment
    expression();
    emit(OpCode::POP);
    consume(Tok::RIGHT_PAREN, "Expect ')' after for clauses.");

    emitLoop(loopStart);

    // at the end of the body, jump to increment, not the condition.
    loopStart = incrementStart;

    patchJump(bodyJump);
  }

  // body
  statement();

  emitLoop(loopStart);

  if (exitJump != -1) {
    patchJump(exitJump);
    emit(OpCode::POP);
  }

  endScope();
}

void Parser::ifStatement() {
  consume(Tok::LEFT_PAREN, "Expect '(' after 'if'");
  
  // TODO: consider strong typing?
  expression();
  consume(Tok::RIGHT_PAREN, "Expect ')' after condition.");

  // jump over the "then" branch if cond is false.
  int elseJump = emitJump(OpCode::JUMP_IF_FALSE);

  // condition.
  emit(OpCode::POP);
  // if POP was executed, the "then" branch will be executed.
  statement();

  // jump over the else branch
  int endJump = emitJump(OpCode::JUMP);
  
  // else branch.
  patchJump(elseJump);
  emit(OpCode::POP);

  if (match(Tok::ELSE)) statement();

  patchJump(endJump);
}

void Parser::printStatement() {
  expression();
  emit((uint8_t)OpCode::PRINT);
  match(Tok::SEMICOLON);
}

void Parser::block() {
  while (!check(Tok::RIGHT_BRACE) && !check(Tok::_EOF)){
    declaration();
  }
  consume(Tok::RIGHT_BRACE, "Expect '}' after block.");
}

void Parser::expressionStatement() {
  expression();
  emit((uint8_t)OpCode::POP);
  match(Tok::SEMICOLON);
}

void Parser::expression() {
  // starts with the lowest kind of precedence.
  parsePrecedence(Precedence::ASSIGNMENT);
}

// the driver for dispatching to sub parsers.
void Parser::parsePrecedence(Precedence prec) {
  // Consumes the starting token.
  advance();
  ParseFn prefixRule = getRule(previous.type).prefix;
  if (prefixRule == nullptr) {
    error("Expect expression.");
    return;
  }

  // As long as precedence is low enough for assignment.
  bool canAssign = prec <= Precedence::ASSIGNMENT;
  prefixRule(canAssign);

  while (prec <= getRule(current.type).precedence) {
    advance();
    ParseFn infixRule = getRule(previous.type).infix;
    infixRule(canAssign);
  }

  // If prefixRule does not consume '=' sign, then no other rules will
  // consume it.
  if (canAssign && match(Tok::EQUAL)) {
    error("Invalid assignment target.");
    expression();
  }
}

void Parser::or_() {
  int elseJump = emitJump(OpCode::JUMP_IF_FALSE);

  // if we reach here, then lhs is true, jump to the end.
  int endJump = emitJump(OpCode::JUMP);

  patchJump(elseJump);

  // pop the lhs.
  emit(OpCode::POP);

  // rhs
  parsePrecedence(Precedence::OR);
  patchJump(endJump);
}

void Parser::and_() {
  // short circuit if the left operand is false
  int endJump = emitJump(OpCode::JUMP_IF_FALSE);

  // remember the lhs has already been parsed & emiited.
  emit(OpCode::POP);

  // parse & emit the rhs.
  parsePrecedence(Precedence::AND);

  // patch JUMP_IF_FALSE's arg.
  patchJump(endJump);
}

uint8_t Parser::makeConstant(Value value) {
  int constant = currentChunk().addConstant(value);

  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constant;
}

ParseRule &Parser::getRule(Tok type) {
  static auto grouping = [this](bool canAssign) { this->grouping(); };
  static auto binary = [this](bool canAssign) { this->binary(); };
  static auto variable = [this](bool canAssign) { this->variable(canAssign); };
  static auto number = [this](bool canAssign) { this->number(); };
  static auto string = [this](bool canAssign) { this->string(); };
  static auto literal = [this](bool canAssign) { this->literal(); };
  static auto unary = [this](bool canAssign) { this->unary(); };
  static auto and_ = [this](bool canAssign) { this->and_(); };
  static auto or_ = [this](bool canAssign) { this->or_(); };

  // The driver of pratt-parsing.
  static ParseRule rules[] = {
    { grouping,    nullptr,    Precedence::CALL },       // TOKEN_LEFT_PAREN
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_RIGHT_PAREN
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_LEFT_BRACE
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_RIGHT_BRACE
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_COMMA
    { nullptr,     nullptr,    Precedence::CALL },       // TOKEN_DOT
    { unary,       binary,     Precedence::TERM },       // TOKEN_MINUS
    { nullptr,     binary,     Precedence::TERM },       // TOKEN_PLUS
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_SEMICOLON
    { nullptr,     binary,     Precedence::FACTOR },     // TOKEN_SLASH
    { nullptr,     binary,     Precedence::FACTOR },     // TOKEN_STAR
    { unary,       nullptr,    Precedence::NONE },       // TOKEN_BANG
    { nullptr,     binary,     Precedence::EQUALITY },   // TOKEN_BANG_EQUAL
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_EQUAL
    { nullptr,     binary,     Precedence::EQUALITY },   // TOKEN_EQUAL_EQUAL
    { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_GREATER
    { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_GREATER_EQUAL
    { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_LESS
    { nullptr,     binary,     Precedence::COMPARISON }, // TOKEN_LESS_EQUAL
    { variable,    nullptr,    Precedence::NONE },       // TOKEN_IDENTIFIER
    { string,      nullptr,    Precedence::NONE },       // TOKEN_STRING
    { number,      nullptr,    Precedence::NONE },       // TOKEN_NUMBER
    { nullptr,     and_,       Precedence::AND },        // TOKEN_AND
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_CLASS
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_ELSE
    { literal,     nullptr,    Precedence::NONE },       // TOKEN_FALSE
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_FOR
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_FUN
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_IF
    { literal,     nullptr,    Precedence::NONE },       // TOKEN_NIL
    { nullptr,     or_,        Precedence::OR },         // TOKEN_OR
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_PRINT
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_RETURN
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_SUPER
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_THIS
    { literal,     nullptr,    Precedence::NONE },       // TOKEN_TRUE
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_VAR
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_WHILE
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_ERROR
    { nullptr,     nullptr,    Precedence::NONE },       // TOKEN_EOF
  };

  return rules[static_cast<int>(type)];
}

void Parser::binary() {
  Tok operatorType = previous.type;

  // parse rhs.
  auto rule = getRule(operatorType);
  parsePrecedence((Precedence)(static_cast<uint8_t>(rule.precedence) + 1));

  switch (operatorType) {
  case Tok::BANG_EQUAL:    emit(OpCode::EQUAL); emit(OpCode::NOT);  break;
  case Tok::EQUAL_EQUAL:   emit(OpCode::EQUAL); break;
  case Tok::GREATER:       emit(OpCode::GREATER); break;
  case Tok::GREATER_EQUAL: emit(OpCode::LESS); emit(OpCode::NOT); break;
  case Tok::LESS:          emit(OpCode::LESS); break;
  case Tok::LESS_EQUAL:    emit(OpCode::GREATER); emit(OpCode::NOT); break;
  case Tok::PLUS:          emit(OpCode::ADD); break;
  case Tok::MINUS:         emit(OpCode::SUBTRACT); break;
  case Tok::STAR:          emit(OpCode::MULTIPLY); break;
  case Tok::SLASH:         emit(OpCode::DIVIDE); break;
  default:
    UNREACHABLE();
  }
}

// prefix for true, false & nil.
void Parser::literal() {
  switch (previous.type) {
  case Tok::FALSE:  emit(OpCode::FALSE);  break;
  case Tok::TRUE:   emit(OpCode::TRUE); break;
  case Tok::NIL:    emit(OpCode::NIL); break;
  default:
    UNREACHABLE();
  }
}

// prefix for Tok::NUMBER.
void Parser::number() {
  double value = strtod(previous.start, nullptr);
  emitConstant(Value(value));
}

// prefix for Tok::STRING.
void Parser::string() {
  // trim the leading and trailing ".

  char *chars = strdup(previous.start + 1);
  chars[previous.length - 2] = '\0';
  ObjectRef s = String::create(vm, chars);

  emitConstant(Value(s, ValueType::String));
}

// prefix for Tok::IDENTIFIER.
void Parser::variable(bool canAssign) {
  Token name = previous;
  OpCode getOp, setOp;
  int arg = resolveLocal(name);
  if (arg != -1) {
    // this is actually a local variable.
    getOp = OpCode::GET_LOCAL;
    setOp = OpCode::SET_LOCAL;
  } else {
    // this is a global variable.
    arg = identifierConstant(name);
    getOp = OpCode::GET_GLOBAL;
    setOp = OpCode::SET_GLOBAL;
  }

  if (canAssign && match(Tok::EQUAL)) {
    // compile as a setter.
    expression();
    emit(setOp);
    emit(arg);
  } else {
    // compile as a getter.
    emit(getOp);
    emit(arg);
  }
}

// prefix for Tok::MINUS & Tok::BANG.
void Parser::unary() {
  Tok operatorType = previous.type;

  parsePrecedence(Precedence::UNARY);
  switch (operatorType) {
  case Tok::BANG: emit(OpCode::NOT); break;
  case Tok::MINUS: emit(OpCode::NEGATE); break;
  default:
    UNREACHABLE();
  }
}

// prefix for Tok::LEFT_PAREN.
void Parser::grouping() {
  expression();
}

void Parser::synchronize() {
  panicMode = false;

  while (current.type != Tok::_EOF) {
    if (previous.type == Tok::SEMICOLON) return;

    switch (current.type) {
    case Tok::CLASS:
    case Tok::FUN:
    case Tok::VAR:
    case Tok::FOR:
    case Tok::IF:
    case Tok::WHILE:
    case Tok::PRINT:
    case Tok::RETURN:
      return;
    default:
      ;
    }
    advance();
  }
}

void Parser::beginScope() {
  scopeDepth++;
}

void Parser::endScope() {
  scopeDepth--;

  // emit bytecode for popping local variables.
  while (locals.count > 0 &&
        locals.back().depth > scopeDepth) {
    emit(OpCode::POP);
    locals.pop();
  }
}

// variable related methods
//

int Parser::resolveLocal(Token &name) {
  for (int i = locals.count - 1; i >= 0; i--) {
    Local &local = locals.get(i);
    if (identifiersEqual(local.name, name)) {
      if (local.depth == -1) {
        error("Cannot reference a local variable before it is initialized.");
      }
      return i;
    }
  }
  // not found.
  return -1;
}

void Parser::declareVariable() {
  if (scopeDepth == 0)  return;

  // make a copy.
  Token name = previous;

  // check for conflicting name in current scope.
  for (int i = locals.count - 1; i >= 0; i--) {
    Local &local = locals.get(i);

    // ensure we've not fallen out of current scope.
    if (local.depth != -1 && local.depth < scopeDepth) break;
    if (identifiersEqual(name, local.name)) {
      error("Variable with this name already declared in this scope.");
    }
  }

  locals.add(name);
}

void Parser::defineVariable(uint8_t global) {
  // The initializer will be on exec stack already.
  if (scopeDepth > 0) {
    // For local variable, no further bytecode is needed.
    markInitialized();
    return;
  }

  // global variables are variables defined in top-level of a module.
  emit(OpCode::DEFINE_GLOBAL);
  emit(global);
}

uint8_t Parser::identifierConstant(Token &name) {
  auto rawNameChars = strdup(name.start);
  rawNameChars[name.length] = '\0';
  auto nameChars = String::create(vm, rawNameChars);
  free(rawNameChars);
  return makeConstant(Value(nameChars, ValueType::String));
}

bool Parser::identifiersEqual(Token &name1, Token &name2) {
  if (name1.length != name2.length) return false;
  return memcmp(name1.start, name2.start, static_cast<size_t>(name1.length)) == 0;
}

void Parser::markInitialized() {
  if (scopeDepth == 0) {
    return;
  }

  locals.set(locals.count - 1, scopeDepth);
}

// state updater
//
void Parser::initParser(Chunk *chunk) {
  _currentChunk = chunk;
  locals.count = 0;
  scopeDepth = 0;
}

void Parser::advance() {
  previous = current;

  while (true) {
    current = scanner.scanToken();
    if (current.type != Tok::ERROR) break;
    errorAtCurrent(current.start);
  }
}

void Parser::consume(Tok type, const char *msg) {
  if (current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(msg);
}

bool Parser::check(Tok type) {
  return current.type == type;
}

bool Parser::match(Tok type) {
  if (!check(type)) return false;
  advance();
  return true;
}

void Parser::endParser() {
#ifdef DEBUG_PRINT_CODE
  if (!hadErorr) {
    // disassembleChunk(currentChunk(), "bytecode");
  }
#endif
  emitReturn();
}

// emitters
//
void Parser::emit(uint8_t byte) {
  currentChunk().write(byte, previous.line);
}

void Parser::emit(OpCode op) {
  currentChunk().write((uint8_t)op, previous.line);
}

void Parser::emitReturn() {
  emit(OpCode::RETURN);
}

void Parser::emitConstant(Value value) {
  emit(OpCode::CONSTANT);
  emit(makeConstant(value));
}

int Parser::emitJump(OpCode jumpInst) {
  // emit the instruction.
  emit(jumpInst);

  // emit the jump offset which can be patched
  emit(0xff);
  emit(0xff);

  // return the index of this offset
  return currentChunk().size() - 2;
}

void Parser::patchJump(int offset) {
  // figure out the number of bytes to jump forward.
  // the preceding two bytes is the jump arg itself, which'll be
  // consumed by the VM.
  int jumpedBytes = currentChunk().size() - offset - 2;

  if (jumpedBytes > UINT16_MAX) {
    error("Too much code to jump over.");
  }

  // write back.
  currentChunk().code[offset] = (jumpedBytes >> 8) & 0xff;
  currentChunk().code[offset + 1] = jumpedBytes & 0xff;
}

// error handling
//
void Parser::error(const char *msg) {
  errorAt(previous, msg);
}

void Parser::errorAtCurrent(const char *msg) {
  errorAt(current, msg);
}

void Parser::errorAt(const Token &token, const char *msg) {
  if (panicMode)  return;
  panicMode = true;

  std::cerr << "[line " << token.line << "] Error";

  if (token.type == Tok::_EOF) {
    std::cerr << " at end";
  } else if (token.type == Tok::ERROR) {}
  else {
    std::cerr << " at '" << token.start;
  }

  std::cerr << "': " << msg << std::endl;
  hadErorr = true;
}

bool Compiler::compileModule(LoxyVM &vm, const char *source, Module &module) {
  Scanner scanner(source);
  Parser parser(vm, scanner);

  return parser.parse(*module.getChunk(), source);
}

} // namespace loxy
