#ifndef loxy_scanner_h
#define loxy_scanner_h

#include "Common.h"

namespace loxy {

// token types
enum class Tok : int {
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
  _EOF,

  TOKEN_NUMS,
};

// a struct that holds enought info for a token.
struct Token {
  Tok         type;
  const char  *start;

  int         length;
  int         line;
};

class Scanner {
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
  bool isAtEnd() const { return *current == '\0'; }
  char advance() {
    current++;
    return current[-1];
  }

  char peek() const { return *current; }

  char peekNext() const {
    if (isAtEnd())  return '\0';
    return current[1];
  }

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

  bool isInitialized() const { return initialized; }

  /// init - helper for initializing [scanner].
  void init(const char *source);
};  // Scanner

} // namespace loxy

#endif