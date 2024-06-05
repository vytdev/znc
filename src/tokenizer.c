#include "lexer.h"
#include "token.h"
#include "types.h"
#include "operator.h"
#include "keyword.h"
#include <stdbool.h>
#include <ctype.h>

const char *TokenTypeNames[] = {
  [TOKEN_EOF]        = "eof",
  [TOKEN_ERROR]      = "error",
  [TOKEN_IDENTIFIER] = "identifier",
  [TOKEN_OPERATOR]   = "operator",
  [TOKEN_KEYWORD]    = "keyword",
  [TOKEN_BRACKET]    = "bracket",
  [TOKEN_DELIMETER]  = "delimeter",
  [TOKEN_STRING]     = "string literal",
  [TOKEN_INTEGER]    = "integer literal",
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
      // a keyword!
      if (iskwd(tok.lexeme) == tok.len)
        tok.type = TOKEN_KEYWORD;
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

    // brackets
    if (
      *lex->lex == '(' || *lex->lex == ')' ||
      *lex->lex == '{' || *lex->lex == '}' ||
      *lex->lex == '[' || *lex->lex == ']'
    ) {
      tok.type = TOKEN_BRACKET;
      tok.len = 1;
      lexer_inc(lex);
      lexer_emit(lex, &tok);
      break;
    }

    // delimeter
    if (*lex->lex == ';') {
      tok.type = TOKEN_DELIMETER;
      tok.len = 1;
      lexer_inc(lex);
      lexer_emit(lex, &tok);
      break;
    }

    // string literal
    if (*lex->lex == '"') {
      tok.type = TOKEN_STRING;
      lexer_inc(lex);
      tok.len++;
      bool escape = false;

      // iterate until the end of the string
      while (
        (escape || *lex->lex != '"') &&
        *lex->lex != '\n' &&
        *lex->lex != '\r' &&
        *lex->lex != '\0'
      ) {

        // escapes (they will be validated at codegen)
        if (escape)
          escape = false;
        else if (*lex->lex == '\\')
          escape = true;

        tok.len++;
        lexer_inc(lex);
      }

      // check the closing quote
      if (*lex->lex != '"') {
        print_token(&tok, "syntax error: unterminated string literal\n");
        tok.type = TOKEN_ERROR;
        lexer_emit(lex, &tok);
        lex->eof = true;
        break;
      }

      // for the closing quote
      lexer_inc(lex);
      tok.len++;
      lexer_emit(lex, &tok);
      break;
    }

    // integer literal
    if (isdigit(*lex->lex)) {
      tok.type = TOKEN_INTEGER;
      while (isdigit(*lex->lex) || *lex->lex == '_') {
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
    print_token(&tok, "syntax error: unknown token\n");
    tok.type = TOKEN_ERROR;
    lexer_emit(lex, &tok);
    lex->eof = true;
    break;
  }
}

