#include "Parser.h"
#include "VM/Value.h"

namespace loxy {

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
  { Parser::grouping, nullptr,        static_cast<int>(Precedence::CALL) },       // Tok::LEFT_PAREN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::RIGHT_PAREN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::LEFT_BRACE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::RIGHT_BRACE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::COMMA
  { nullptr,          nullptr,        static_cast<int>(Precedence::CALL) },       // Tok::DOT
  { Parser::unary,    Parser::binary, static_cast<int>(Precedence::TERM) },       // Tok::MINUS
  { nullptr,          Parser::binary, static_cast<int>(Precedence::TERM) },       // Tok::PLUS
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::SEMICOLON
  { nullptr,          Parser::binary, static_cast<int>(Precedence::FACTOR) },     // Tok::SLASH
  { nullptr,          Parser::binary, static_cast<int>(Precedence::FACTOR) },     // Tok::STAR
  { Parser::unary,    nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::BANG
  { nullptr,          Parser::binary, static_cast<int>(Precedence::EQUALITY) },   // Tok::BANG_EQUAL
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::EQUAL
  { nullptr,          Parser::binary, static_cast<int>(Precedence::EQUALITY) },   // Tok::EQUAL_EQUAL
  { nullptr,          Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::GREATER
  { nullptr,          Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::GREATER_EQUAL
  { nullptr,          Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::LESS
  { nullptr,          Parser::binary, static_cast<int>(Precedence::COMPARISON) }, // Tok::LESS_EQUAL
  { Parser::variable, nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::IDENTIFIER
  { Parser::string,   nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::STRING
  { Parser::number,   nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::NUMBER
  { nullptr,          Parser::and_,   static_cast<int>(Precedence::AND) },        // Tok::AND
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::CLASS
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::ELSE
  { Parser::atom,     nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::FALSE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::FOR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::FUN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::IF
  { Parser::atom,     nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::NIL
  { nullptr,          Parser::or_,    static_cast<int>(Precedence::OR) },         // Tok::OR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::PRINT
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::RETURN
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::SUPER
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::THIS
  { Parser::atom,     nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::TRUE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::VAR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::WHILE
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::ERROR
  { nullptr,          nullptr,        static_cast<int>(Precedence::NONE) },       // Tok::EOF
};

uint8_t Parser::makeConstant(Value value) {
  int constant = currentChunk().addConstant(value);

  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }
  return (uint8_t)constant;
}

uint8_t Parser::parseVariable(const char *msg) {

}

// driver
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

void Parser::statement() {
  if (match(Tok::FOR)) {
    forStatement();
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