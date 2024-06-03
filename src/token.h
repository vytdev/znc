#ifndef _ZNC_TOKEN_H
#define _ZNC_TOKEN_H

// token types
typedef enum {
  TOKEN_EOF,     // end-of-file
  TOKEN_ERROR,   // lexer error
} TokenType;

// token type names
extern const char *TokenTypeNames[];

#endif // _ZNC_TOKEN_H

