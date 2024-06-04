#ifndef _ZNC_LEXER_H
#define _ZNC_LEXER_H
#include "types.h"
#include "token.h"
#include <stdbool.h>

typedef struct {
  struct Lexer *lexer;  /* ptr to the source lexer */

  TokenType type;       /* type of the token */
  char *lexeme;         /* token text string view */

  uvar len;             /* length of the token */
  uvar line;            /* line loc of the token */
  uvar col;             /* col loc of the token */
  uvar pos;             /* pos loc of the token */
} Token;

typedef struct Lexer {
  char *name;           /* name of the lexer */

  char *input;          /* input text */
  char *lex;            /* current char on lexer */
  uvar len;             /* length of input */
  bool eof;             /* whether the lexer has reached the end of input */

  uvar line;            /* current line */
  uvar col;             /* current col */
  uvar pos;             /* current pos */

  uvar pind;            /* lexer position indicator (to next token) */
  Token *toks;          /* array of tokens */
  uvar talloc;          /* allocation size of toks */
  uvar tcnt;            /* number of emitted tokens */
} Lexer;

/* initialize a lexer */
int lexer_init(Lexer *lex, char *name, char *src);

/* free a lexer context */
void lexer_free(Lexer *lex);

/* increment the lexer counter */
void lexer_inc(Lexer *lex);

/* emit a token */
void lexer_emit(Lexer *lex, Token *tok);

/* get next token and advance the lexer */
Token *lexer_consume(Lexer *lex);

/* get next token, but do not advance the lexer */
Token *lexer_peek(Lexer *lex, var offst);

/* move the position indicator */
void lexer_seek(Lexer *lex, var offst);

/* print a token */
void print_token(Token *tok, const char *msg, ...);

/* expect a token, returns 0 if succeded, 1 otherwise */
int expect_token(Token *tok, TokenType type, char *text);

/* compare token to given type and text, returns true if match */
int cmp_token(Token *tok, TokenType type, char *text);

/* process next tokens */
void lexer_tokenize(Lexer *lex);

#endif // _ZNC_LEXER_H

