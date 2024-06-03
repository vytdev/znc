#include "ast.h"
#include "lexer.h"
#include "arena.h"

ASTIdentifier *ast_identifier(Lexer *lex, Arena *arena) {
  ASTIdentifier *node = aaloc(arena, ASTIdentifier);
  if (!node)
    return NULL;

  // consume token, check if this is an identifier
  Token *tok = lexer_consume(lex);
  if (expect_token(tok, TOKEN_IDENTIFIER, NULL))
    return NULL;

  // set some fields
  node->name = tok->lexeme;
  node->len = tok->len;

  return node;
}

