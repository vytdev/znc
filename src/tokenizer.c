#include "lexer.h"
#include "token.h"
#include <stdbool.h>
#include <ctype.h>

const char *TokenTypeNames[] = {
  [TOKEN_EOF]        = "eof",
  [TOKEN_ERROR]      = "error",
  [TOKEN_IDENTIFIER] = "identifier",
};

void lexer_tokenize(Lexer *lex) {
  while (!lex->eof) {
    // skip whitespace
    while (
        (*lex->lex == ' ' || *lex->lex == '\t' || *lex->lex == '\n' ||
        *lex->lex == '\r') && *lex->lex != '\0'
    ) lexer_inc(lex);

    // eof
    if (*lex->lex == '\0') {
      Token tok = {
        .lexer  = lex,
        .type   = TOKEN_EOF,
        .lexeme = lex->lex,
        .len    = 0,
        .line   = lex->line,
        .col    = lex->col,
        .pos    = lex->pos,
      };
      lexer_emit(lex, &tok);
      lex->eof = true;
      return;
    }

    // skip single line comments
    if (*lex->lex == '/' && *(lex->lex + 1) == '/') {
      while (*lex->lex != '\n' && *lex->lex != '\0')
        lexer_inc(lex);
      continue;
    }

    // skip multiline comments
    if (*lex->lex == '/' && *(lex->lex + 1) == '*') {
      while ((*lex->lex != '*' || *(lex->lex + 1) != '/') && *lex->lex != '\0')
        lexer_inc(lex);
      lexer_inc(lex); // end '*'
      lexer_inc(lex); // end '/'
      continue;
    }

    // start token
    Token tok = {
      .lexer  = lex,
      .type   = TOKEN_ERROR,
      .lexeme = lex->lex,
      .len    = 0,
      .line   = lex->line,
      .col    = lex->col,
      .pos    = lex->pos,
    };

    // identifier token
    if (isalpha(*lex->lex)) {
      tok.type = TOKEN_IDENTIFIER;
      while (isalnum(*lex->lex)) {
        tok.len++;
        lexer_inc(lex);
      }
      lexer_emit(lex, &tok);
      break;
    }

    // unknown token
    while (
      *lex->lex != ' ' && *lex->lex != '\t' && *lex->lex != '\n' &&
      *lex->lex != '\r' && *lex->lex != '\0'
    ) {
      tok.len++;
      lexer_inc(lex);
    }
    print_token(&tok, "unknown token\n");
    tok.type = TOKEN_ERROR;
    lexer_emit(lex, &tok);
    lex->eof = true;
    break;
  }
}

