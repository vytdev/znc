#ifndef _ZNC_TOKEN_H
#define _ZNC_TOKEN_H
#include "types.h"

// token types
typedef enum {
  TOKEN_EOF,     // end-of-file
  TOKEN_ERROR,   // lexer error
  TOKEN_IDENTIFIER,
  TOKEN_OPERATOR,
  TOKEN_BRACKET,
} TokenType;

// token type names
extern const char *TokenTypeNames[];

#endif // _ZNC_TOKEN_H

