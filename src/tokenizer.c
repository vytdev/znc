#include "lexer.h"
#include "token.h"
#include "types.h"
#include "operator.h"
#include <stdbool.h>
#include <ctype.h>

const char *TokenTypeNames[] = {
  [TOKEN_EOF]        = "eof",
  [TOKEN_ERROR]      = "error",
  [TOKEN_IDENTIFIER] = "identifier",
  [TOKEN_OPERATOR]   = "operator",
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
    if (isalpha(*lex->lex) || *lex->lex == '_') {
      tok.type = TOKEN_IDENTIFIER;
      while (isalnum(*lex->lex) || *lex->lex == '_') {
        tok.len++;
        lexer_inc(lex);
      }
      lexer_emit(lex, &tok);
      break;
    }

    // operators
    uvar oplen = isop(lex->lex);
    if (oplen > 0) {
      tok.type = TOKEN_OPERATOR;
      tok.len = oplen;
      lex->lex += oplen;
      lex->col += oplen;
      lex->pos += oplen;
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

