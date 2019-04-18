#include <cstring>
#include "Scanner.h"

namespace loxy {

//--=== helpers ===--//

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

} // namespace loxy