#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include "Parser.h"
#include "VM/Value.h"

namespace loxy {

Parser::Parser(VM &vm)
  : scanner_(nullptr), vm(vm),
    hadError(false), panicMode(false),
    currentChunk_(nullptr), currentFunc_(nullptr) {}

bool Parser::parse(Chunk *compilingChunk, const char *source) {
  currentChunk_ = compilingChunk;

  // create a scanner.
  Scanner scanner(source);
  scanner_ = &scanner;

  // begin a new function here.
  FunctionScope function;
  beginFunction(&function);

  // initialize [current] with the first token.
  advance();

  while (!match(Tok::_EOF)) {
    declaration();
  }
  
  return !hadError;
}

static std::string printSrc(const Token &token, int idents) {
  std::stringstream ss;

  for (int i = 0; i < idents; i++) ss << "\t";

  if (token.type == Tok::_EOF) {
    ss << "end of the file";
    return ss.str();
  }

  for (int i = 0; token.start[i] != '\0' && token.start[i] != '\n'; i++) {
    ss << token.start[i];
  }
  return ss.str();
}

void Parser::errorAt(const Token &token, const char *msg) {
  // don't report errors found in panic mode.
  if (panicMode)  return;

  panicMode = true;

  std::cerr << "[line " << token.line 
    << "] compilation error at\n"
    << printSrc(token, 1) << "\n"
    << msg << "\n";
}

void Parser::error(const char *msg) {
  errorAt(previous, msg);
}

void Parser::errorAtCurrent(const char *msg) {
  errorAt(current, msg);
}

void Parser::synchronize() {
  panicMode = false;

  while (current.type != Tok::_EOF) {
    if (previous.type == Tok::SEMICOLON)  return;

    switch (current.type)
    {
    // these keywords starts a new declaration/statement.
    case Tok::CLASS:
    case Tok::FUN:
    case Tok::VAR:
    case Tok::FOR:
    case Tok::IF:
    case Tok::WHILE:
    case Tok::PRINT:
    case Tok::RETURN:
      return;
    
    default:  ;
    }
    advance();
  }
}

// precedence of expression.
enum class Precedence : int {
  NONE,        // lowest precedence
  ASSIGNMENT,  // =
  
  //--=== things below are not assignable ===--//
  OR,          // or
  AND,         // and
  EQUALITY,    // == !=
  COMPARISON,  // < > <= >=
  TERM,        // + -
  FACTOR,      // * /
  UNARY,       // ! - +
  CALL,        // . () []
  PRIMARY      // should be removed?
};

Parser::ParseRule Parser::rules[] = {
  { &Parser::grouping, nullptr,        static_cast<int>(Precedence::CALL) },       // Tok::LEFT_PAREN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::RIGHT_PAREN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::LEFT_BRACE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::RIGHT_BRACE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::COMMA
  { nullptr,          nullptr,        static_cast<int>(Precedence::CALL) },       // Tok::DOT
  { &Parser::unary,   &Parser::binary, static_cast<int>(Precedence::TERM) },       // Tok::MINUS
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::TERM) },       // Tok::PLUS
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::SEMICOLON
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::FACTOR) },     // Tok::SLASH
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::FACTOR) },     // Tok::STAR
  { &Parser::unary,    nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::BANG
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::EQUALITY) },   // Tok::BANG_EQUAL
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::EQUAL
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::EQUALITY) },   // Tok::EQUAL_EQUAL
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::GREATER
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::GREATER_EQUAL
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::LESS
  { nullptr,          &Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::LESS_EQUAL
  { &Parser::variable, nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::IDENTIFIER
  { &Parser::string,   nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::STRING
  { &Parser::number,   nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::NUMBER
  { nullptr,          &Parser::and_,   static_cast<int>(Precedence::AND) },        // Tok::AND
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::CLASS
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::ELSE
  { &Parser::atom,     nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::FALSE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::FOR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::FUN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::IF
  { &Parser::atom,     nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::NIL
  { nullptr,          &Parser::or_,    static_cast<int>(Precedence::OR) },         // Tok::OR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::PRINT
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::RETURN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::SUPER
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::THIS
  { &Parser::atom,     nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::TRUE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::VAR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::WHILE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::ERROR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::EOF
};

// helpers for updating parser's internal state
//
void Parser::advance() {
  previous = current;

  while (true) {
    current = scanner_->scanToken();
    if (current.type != Tok::ERROR) break;
    errorAtCurrent(current.start);
  }
}

void Parser::consume(Tok type, const char *msg) {
  if (check(type)) {
    advance();
    return;
  }

  errorAtCurrent(msg);
}

bool Parser::check(Tok type) {
  return current.type == type;
}

bool Parser::match(Tok type) {
  if (check(type)) { advance(); return true; }
  return false;
}

uint8_t Parser::makeConstant(Value value) {
  int constant = currentChunk().addConstant(value);

  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constant;
}

// only global variable names are stored.
uint8_t Parser::identifierConstant(Token name) {
  String *identifier = String::create(vm, name.start, name.length);
  return makeConstant(Value(identifier, ValueType::String));
}

bool Parser::identifiersEqual(const Token &a, const Token &b) {
  assert(a.type == Tok::IDENTIFIER && b.type == Tok::IDENTIFIER && "Comparing identifiers!");
  
  if (a.length != b.length) return false;

  return memcmp(a.start, b.start, a.length) == 0;
}

// parse a variable name 
uint8_t Parser::declareVariable(const char *msg) {
  consume(Tok::IDENTIFIER, msg);

  if (currentFunc_->depth == 0) return declareGlobal();

  return declareLocal();
}

uint8_t Parser::declareGlobal() {
  return identifierConstant(previous);
}

uint8_t Parser::declareLocal() {
  Token name = previous;
  FunctionScope *function = currentFunc_;
  int depth = function->depth;

  // checks for conflicts.
  for (int i = function->count - 1; i >= 0; i--) {
    const Variable &var = function->getLocal(i);

    // shadowing is fine.
    if (var.depth < depth)  break;
    if (identifiersEqual(name, var.name)) {
      // TODO: print this name
      error("variable with this name has already been declared in this scope");
    }
  }

  // mark as declared.
  function->createLocal(name);
  return 0;
}

int Parser::resolveLocal(const Token &name) {
  for (int i = currentFunc_->count; i >= 0; i--) {
    if (identifiersEqual(currentFunc_->getLocal(i).name, name)) {
      if (currentFunc_->getLocal(i).depth == -1) {
        error("cannot read an uninitialized local variable");
      }
      return i;
    }
  }
  return -1;
}

void Parser::defineVariable(uint8_t var) {
  // global
  if (currentFunc_->depth == 0) {
    emit(OpCode::DEFINE_GLOBAL);
    emit(var);  // index into the current chunk's constant table.
  } else {
    // mark the local variable as initialized.
    currentFunc_->vars[currentFunc_->count - 1].depth = currentFunc_->depth;
  }
}

// emitters
void Parser::emit(uint8_t byte) {
  currentChunk().write(byte, previous.line);
}

void Parser::emit(OpCode op) {
  emit(static_cast<uint8_t>(op));
}

void Parser::emitReturn() {
  // TODO:
  // implicit return for initializer

  emit(OpCode::RETURN);
}

void Parser::emitConstant(Value value) {
  emit(OpCode::CONSTANT);
  // the index into the currentChunk's constant pool.
  emit(makeConstant(value));
}

int Parser::emitJump(OpCode jumpInst) {
  emit(jumpInst);
  // args of type uint16_t
  emit(0xff);
  emit(0xff);
  // returns the index into the bytecode for this 
  return currentChunk().size() - 2;
}

void Parser::emitLoop(int loopStart) {
  emit(OpCode::LOOP);

  // calculate the offset from current to [loopStart]
  // such that when vm encounters OpCode::LOOP, it simply
  // reads the next 2 bytes as the offset and subtract it by
  // the instruction pointer.

  // the extra 2 bytes are reserved for arg of OpCode::LOOP
  int offset = currentChunk().size() + 2 - loopStart;
  if (offset > UINT16_MAX)  error("loop body too large");

  // big endian
  emit((offset >> 8) & 0xff);
  emit(offset & 0xff);
}

void Parser::patchJump(int index) {
  // the index points to the start of the first byte of OpCode::JUMP's
  // arg. When the vm encounters OpCode::JUMP or OpCode::JUMP_IF_FALSE,
  // it has already advanced the instruction pointer 2 bytes further.
  int offset = currentChunk().size() - index - 2;
  if (offset > UINT16_MAX)  error("jump too far");

  // big endian
  currentChunk().code[index] = (offset >> 8) & 0xff;
  currentChunk().code[index + 1] = offset &0xff;
}

// driver for expressions
// called on previous token that was consumed & it must be
// a prefix expression.
void Parser::parsePrecedence(int prec) {
  advance();
  int ruleIndex = static_cast<int>(previous.type);

  // prefix
  ParseFn prefix = rules[ruleIndex].prefix;
  if (prefix == nullptr) {
    error("expect expression");
    return;
  }

  // this is to prevent something like
  // 1 + 2 = "a string".
  // this means only when prec is NONE or ASSIGNMENT, can
  // there be an equal sign on the next.
  // basically, only parser "variable" will use this value.
  bool canAssign = prec <= static_cast<int>(Precedence::ASSIGNMENT);

  // [prefix] here is the sub recursive descent parser here,
  // which can consume as many tokens as it wants.
  (this->*prefix)(canAssign);

  // as long as the passed in arg [prec] is smaller than next token's
  // precedence, we'll just consume it as part of the subtree.
  while (prec <= rules[static_cast<int>(current.type)].precedence) {
    advance();
    ParseFn infix = rules[static_cast<int>(previous.type)].infix;
    (this->*infix)(canAssign);
  }

  // this case happens when you're trying to assign to
  // a string or number or atom literal.
  if (canAssign && match(Tok::EQUAL)) {
    error("Invalid assignment target");
    expression();
  }
}

void Parser::declaration() {
  if (match(Tok::VAR)) {
    varDeclaration();
  } else {
    // top-level statement
    statement();
  }

  if (panicMode)  synchronize();
}

void Parser::varDeclaration() {
  // parse & declare this variable.
  uint8_t global = declareVariable("expect variable name");

  // initializer
  if (match(Tok::EQUAL)) {
    expression();
  } else {
    emit(OpCode::NIL);
  }

  // optional
  match(Tok::SEMICOLON);

  // define this variable.
  defineVariable(global);
}

void Parser::statement() {
  if (match(Tok::FOR)) {
    forStatement();
  } else if (match(Tok::WHILE)) {
    whileStatement();
  } else if (match(Tok::IF)) {
    ifStatement();
  } else if (match(Tok::LEFT_BRACE)) {
    // push a new lexical scope
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

void Parser::forStatement() {
  beginScope();

  consume(Tok::LEFT_PAREN, "expect '(' after 'for'");

  // init statement
  if (match(Tok::VAR))  varDeclaration();
  else if (match(Tok::SEMICOLON)) {

  } else {
    // expressionStatement optionally consumes ';'.
    // but this is a must here.
    expression();
    consume(Tok::SEMICOLON, "expect ';' after initial ");
    emit(OpCode::POP);
  }

  // remember the position of the start of the loop such that
  // we can jump back here later.
  // loops always start at conditions.
  int loopStart = currentChunk().size();

  // remember the position of the start of the increment such that
  // we can jump back here later.
  int incrStart;

  // condition.
  if (!check(Tok::SEMICOLON)) {
    expression();
  } else {
    // if no condition expression is specified,
    // treat it as an infinite loop.
    emit(OpCode::TRUE);
  }

  consume(Tok::SEMICOLON, "expect ';' after condition");

  int exitLoop = emitJump(OpCode::JUMP_IF_FALSE);
  
  // pop condition.
  emit(OpCode::POP);

  // increment.
  if (!check(Tok::RIGHT_PAREN)) {
    incrStart = currentChunk().size();
    expression();
    emit(OpCode::POP);
    // loop back to condition
    emitLoop(loopStart);
  } else {
    // points back to condition instead.
    incrStart = loopStart;
  }

  consume(Tok::RIGHT_PAREN, "expect ')' after 'for' loop");

  // body
  statement();

  // always loop to increment
  emitLoop(incrStart);

  // exit
  patchJump(exitLoop);

  // pop condition.
  emit(OpCode::POP);
  endScope();
}

void Parser::whileStatement() {
  // remember the position of loop start.
  // loops always start at condition.  
  int loopStart = currentChunk().size();
  consume(Tok::LEFT_PAREN, "expect '(' after 'while'");
  // condition.
  expression();
  consume(Tok::RIGHT_PAREN, "expect')' after while condition");

  int exitJump = emitJump(OpCode::JUMP_IF_FALSE);
  // pop condition.
  emit(OpCode::POP);

  // body.
  statement();
  emitLoop(loopStart);

  patchJump(exitJump);
  // pop condition.
  emit(OpCode::POP);
}

void Parser::ifStatement() {
  consume(Tok::LEFT_PAREN, "expect '(' after 'if'");
  
  // condition
  expression();
  consume(Tok::RIGHT_PAREN, "expect ')' after condition");

  int elseJump = emitJump(OpCode::JUMP_IF_FALSE);

  // pop condition.
  emit(OpCode::POP);

  // then body.
  statement();

  // jump over the else body.
  int endJump = emitJump(OpCode::JUMP);

  // if condition is false, jump to bytecode below.
  patchJump(elseJump);

  // pop condition.
  emit(OpCode::POP);

  // else body
  if (match(Tok::ELSE)) {
    statement();
  }
  patchJump(endJump);
}

void Parser::block() {
  while (!check(Tok::RIGHT_BRACE)) {
    declaration();
  }

  consume(Tok::RIGHT_BRACE, "expect '}' after block");
}

void Parser::expressionStatement() {
  expression();
  match(Tok::SEMICOLON);
  emit(OpCode::POP);
}

// printStatement := "print" expression ;
void Parser::printStatement() {
  emit(OpCode::PRINT);
  expression();
  match(Tok::SEMICOLON);
  emit(OpCode::POP);
}

// parse from lowest possible precedence expression
void Parser::expression() {
  parsePrecedence(static_cast<int>(Precedence::ASSIGNMENT));
}

// infix
void Parser::or_(bool _) {
  int elseJump = emitJump(OpCode::JUMP_IF_FALSE);
  
  // ends with lhs
  int endJump = emitJump(OpCode::JUMP);

  patchJump(elseJump);

  emit(OpCode::POP);

  // rhs
  parsePrecedence(static_cast<int>(Precedence::OR));
  patchJump(endJump);
}

// infix
void Parser::and_(bool _) {
  // if the lhs is falsy, jump to end
  int jumpArg = emitJump(OpCode::JUMP_IF_FALSE);

  // pop lhs
  emit(OpCode::POP);

  // parse & emit rhs
  parsePrecedence(static_cast<int>(Precedence::AND));

  // 'and' evaluates to the rhs, so don't pop it.

  // patch the arg to jump over all bytecode emitted by [parsePrecedence].
  patchJump(jumpArg);
}

// infix
void Parser::binary(bool _) {
  Tok op = previous.type;

  // this is to ensure the current binary expression
  // does not contain lower precedent binary expression.
  int precedence = rules[static_cast<int>(previous.type)].precedence;
  parsePrecedence(precedence);

  switch (op)
  {
  case Tok::BANG_EQUAL:     emit(OpCode::EQUAL); emit(OpCode::NOT); break;
  case Tok::EQUAL_EQUAL:    emit(OpCode::EQUAL); break;
  case Tok::GREATER:        emit(OpCode::GREATER); break;
  case Tok::GREATER_EQUAL:  emit(OpCode::LESS); emit(OpCode::NOT); break;
  case Tok::LESS:           emit(OpCode::LESS); break;
  case Tok::LESS_EQUAL:     emit(OpCode::GREATER); emit(OpCode::NOT); break;
  case Tok::PLUS:           emit(OpCode::ADD); break;
  case Tok::MINUS:          emit(OpCode::SUBTRACT); break;
  case Tok::STAR:           emit(OpCode::MULTIPLY); break;
  case Tok::SLASH:          emit(OpCode::DIVIDE); break;
  default:                  UNREACHABLE();
  }
}

// grouping := '(' expression ')' ;
void Parser::grouping(bool canAssign) {
  expression();
  consume(Tok::RIGHT_PAREN, "expect ')' after expression");
}

// unary := ('!' | '-') (call | primary) ;
void Parser::unary(bool _) {
  Tok op = previous.type;

  // inner expression.
  parsePrecedence(static_cast<int>(Precedence::UNARY));
  switch (op)
  {
  case Tok::BANG:   emit(OpCode::NOT); break;
  case Tok::MINUS:  emit(OpCode::NEGATE); break;
  default:          UNREACHABLE();
  }
}

// variable := getVar | setVar ;
// getVar := identifier ;
// setVar := identifier '=' expression ;
void Parser::variable(bool assignable) {
  // this is where you might wanna consume the equal sign.
  Token name = previous;
  OpCode getOp, setOp;
  int index = resolveLocal(name);

  if (index == -1) {
    getOp = OpCode::GET_GLOBAL;
    setOp = OpCode::SET_GLOBAL;
  } else {
    // find the index of [name] in vm's global symbol table.
    index = identifierConstant(name);
    getOp = OpCode::GET_LOCAL;
    setOp = OpCode::SET_LOCAL;
  }

  if (assignable && match(Tok::EQUAL)) {
    // parse the assigned expression & emit it first.
    expression();
    emit(setOp);
    emit(index);
  } else {
    emit(getOp);
    emit(index);
  }
}

// primary
void Parser::string(bool _) { 
  String *str = String::create(vm, previous.start, previous.length);

  // emit the string on the stack.
  emitConstant(Value(str, ValueType::String));
}

// primary
void Parser::number(bool _) {
  double value = strtod(previous.start, nullptr);
  
  // emit the number on the stack.
  emitConstant(Value(value));
}

// primary
void Parser::atom(bool _) {
  switch (previous.type)
  {
  case Tok::FALSE:  emit(OpCode::FALSE); break;
  case Tok::TRUE:   emit(OpCode::TRUE);  break;
  case Tok::NIL:    emit(OpCode::NIL);   break;
  default:          UNREACHABLE();
  }
}

} // namespace loxy