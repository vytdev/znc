#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

int lexer_init(Lexer *lex, char *name, char *src) {
  if (!lex)
    return 1;
  // general
  lex->name  = name;
  lex->input = src;
  lex->lex   = src;
  lex->len   = strlen(src);
  lex->eof   = false;

  // counters
  lex->line  = 1;
  lex->col   = 1;
  lex->pos   = 0;

  // token trackers
  lex->pind  = 0;
  lex->toks  = (Token*)malloc(sizeof(Token));
  if (!lex->toks)
    return 1;
  lex->talloc = 1;
  lex->tcnt  = 0;

  return 0;
}

void lexer_free(Lexer *lex) {
  if (!lex)
    return;
  lex->name  = NULL;
  lex->input = NULL;
  lex->lex   = NULL;
  if (lex->toks) {
    free(lex->toks);
    lex->toks = NULL;
  }
  return;
}

void lexer_inc(Lexer *lex) {
  if (!lex || lex->eof)
    return;

  // increment counters based on chars
  lex->pos++;
  switch (*lex->lex++) {
    case '\r':
      lex->col = 1;
      break;
    case '\n':
      lex->col = 1;
      lex->line++;
      break;
    case '\t':
      // +1 because col starts at 1
      lex->col += 8 - (lex->col % 8) + 1;
      break;
    default:
      lex->col++;
  }

  return;
}

void lexer_emit(Lexer *lex, Token *tok) {
  if (!lex || !tok)
    return;

  // already eof, you cannot emit more tokens
  if (lex->eof)
    return;

  // the list is full, re-allocate it!
  if (lex->talloc <= lex->tcnt) {
    Token *tmp = (Token*)realloc(lex->toks, sizeof(Token) * lex->talloc * 2);
    if (!tmp) {
      fprintf(stderr, "znc: out of memory\n");
      lex->eof = true;
      return;
    }
    lex->toks = tmp;
    lex->talloc *= 2;
  }

  // copy tok into the list
  Token *out = &lex->toks[lex->tcnt++];
  out->lexer  = lex;
  out->type   = tok->type;
  out->lexeme = tok->lexeme;
  out->len    = tok->len;
  out->line   = tok->line;
  out->col    = tok->col;
  out->pos    = tok->pos;
  return;
}

Token *lexer_consume(Lexer *lex) {
  if (!lex)
    return NULL;

  // process next tokens if needed
  while (lex->tcnt <= lex->pind && !lex->eof)
    lexer_tokenize(lex);

  // maybe eof?
  if (lex->tcnt <= lex->pind) {
    if (!lex->eof) return NULL;
    return &lex->toks[lex->tcnt - 1];
  }

  // return the requested token
  return &lex->toks[lex->pind++];
}

Token *lexer_peek(Lexer *lex, var offst) {
  if (!lex)
    return NULL;

  // get the absolute pos from current pos indicator and offset,
  // -1 because lex->pind always points to the token
  uvar absp = lex->pind + offst - 1;

  // negative absolute position... out of bound access
  if (offst < 1 && lex->pind < absp)
    return NULL;

  // process next tokens if not enough
  while (lex->tcnt <= absp && !lex->eof)
    lexer_tokenize(lex);

  // lexer_tokenize() didn't reached the target position, eof?
  if (lex->tcnt <= absp) {
    if (!lex->eof) return NULL;
    return &lex->toks[lex->tcnt - 1];
  }

  // the requested token
  return &lex->toks[absp];
}

void lexer_seek(Lexer *lex, var offst) {
  if (!lex)
    return;
  lex->pind += offst;
  return;
}

void print_token(Token *tok, const char *msg, ...) {
  if (!tok || !msg)
    return;

  // message
  printf("%s:%u:%u: ", tok->lexer->name, tok->line, tok->col);
  va_list args;
  va_start(args, msg);
  vprintf(msg, args);
  va_end(args);

  // get the line number print length
  uvar lntmp = tok->line;
  uvar len = 0;
  do {
    len++;
    lntmp /= 10;
  } while (lntmp != 0);

  // now, find the start of the line
  char *lstart = tok->lexeme;
  while (tok->lexer->input < lstart && *(lstart - 1) != '\n')
    lstart--;

  // print!
  printf("  %u | ", tok->line);
  uvar currCol = 0;
  while (*lstart != '\r' && *lstart != '\n' && *lstart != '\0') {
    char ch = *lstart++;

    // handle tabs
    if (ch == '\t'){
      int width = 8 - (currCol % 8);
      for (int i = 0; i < width; i++)
        fputc(' ', stdout);
      currCol += width;
      continue;
    }

    // other normal chars
    fputc(ch, stdout);
    currCol++;
  }
  fputc('\n', stdout);

  // print arrows
  printf("  ");
  for (int i = 0; i < len; i++)
    fputc(' ', stdout);
  printf(" | ");
  for (int i = 0; i < tok->col - 1; i++)
    fputc(' ', stdout);
  for (int i = 0; i < tok->len; i++)
    fputc('^', stdout);
  fputc('\n', stdout);
}

int expect_token(Token *tok, TokenType type, char *text) {
  if (!tok)
    return 1;
  if (cmp_token(tok, type, text))
    return 0;

  if (tok->type == TOKEN_ERROR)
    return 1; // the tokenizer already printed the error
  if (tok->type == TOKEN_EOF)
    print_token(tok, "syntax error: unexpected end of input\n");
  else if (text)
    print_token(tok, "syntax error: expected '%s'\n", text);
  else
    print_token(tok, "syntax error: unexpected token\n");

  return 1;
}

int cmp_token(Token *tok, TokenType type, char *text) {
  if (!tok)
    return 0;
  return tok->type == type && (text
     ? strncmp(text, tok->lexeme, tok->len) == 0
     : 1
  );
}

