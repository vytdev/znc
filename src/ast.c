#include "ast.h"
#include "lexer.h"
#include "arena.h"
#include "token.h"
#include "operator.h"
#include "util.h"
#include <stdio.h>

// NOTES:
// - only use valptr() on aaloc, lexer_peek, and lexer_consume outputs
// - it is not necessary to check the 'lex' and 'arena' ptr

ASTExpr *parse_identifier(Lexer *lex, Arena *arena) {
  ASTExpr *node = aaloc(arena, ASTExpr);
  if (valptr(node)) return NULL;

  // consume token, check if this is an identifier
  Token *tok = lexer_consume(lex);
  if (valptr(tok))
    return NULL;

  if (expect_token(tok, TOKEN_IDENTIFIER, NULL))
    return NULL;

  // set some fields
  node->type = AST_EXPR_IDENTIFIER;
  node->val.ident.name = tok->lexeme;
  node->val.ident.len = tok->len;

  return node;
}

ASTExpr *parse_expr(Lexer *lex, Arena *arena) {
  return parse_infix(lex, arena, parse_factor(lex, arena), 1);
}

ASTExpr *parse_infix(Lexer *lex, Arena *arena, ASTExpr *lhs, int minprec) {
  if (!lhs) return NULL;

  // ensure the left hand side is processed
  if (minprec > OP_HIGHEST_PREC) return lhs;
  lhs = parse_infix(lex, arena, lhs, minprec + 1);

  // get next token
  Token *next = lexer_peek(lex, 1);
  if (valptr(next)) return NULL;

  // while token
  while (next->type == TOKEN_OPERATOR && getprec(getop(next->lexeme)) >= minprec) {
    lexer_consume(lex);
    OperatorType op = getop(next->lexeme);

    // process right hand side operand
    ASTExpr *rhs = parse_factor(lex, arena);
    if (!rhs) return NULL;

    // process higher-precedence tokens
    next = lexer_peek(lex, 1);
    if (valptr(next)) return NULL;
    while (next->type == TOKEN_OPERATOR && getprec(getop(next->lexeme)) > minprec) {
      rhs = parse_infix(lex, arena, rhs, minprec + 1);
      next = lexer_peek(lex, 1);
     if (valptr(next)) return NULL;
    }

    // new expr
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (valptr(node)) return NULL;
    node->type = AST_EXPR_BINOP;
    node->val.binop.lhs = lhs;
    node->val.binop.op = op;
    node->val.binop.rhs = rhs;
    lhs = node;
 }

  return lhs;
}

ASTExpr *parse_factor(Lexer *lex, Arena *arena) {
  Token *next = lexer_peek(lex, 1);
  if (valptr(next))
    return NULL;

  // identifier token
  if (next->type == TOKEN_IDENTIFIER)
    return parse_identifier(lex, arena);

  // expression enclosed with paren
  if (next->type == TOKEN_BRACKET && *next->lexeme == '(') {
    lexer_consume(lex);
    ASTExpr *expr = parse_infix(lex, arena, parse_factor(lex, arena), 1);
    if (!expr) return NULL;
    next = lexer_consume(lex);
    if (valptr(next)) return NULL;
    if (expect_token(next, TOKEN_BRACKET, ")")) return NULL;
    return expr;
  }

  // unary operators
  // TODO

  print_token(next, "syntax error: unexpected token\n");
  return NULL;
}

#ifdef _DEBUG
void print_expr(ASTExpr *expr) {
  if (!expr) {
    printf("[null]");
    return;
  }

  fputc('(', stdout);
  switch (expr->type) {
    case AST_EXPR_BINOP:
      print_expr(expr->val.binop.lhs);
      printf(" %s ", OperatorNames[expr->val.binop.op]);
      print_expr(expr->val.binop.rhs);
      break;
    case AST_EXPR_IDENTIFIER:
      pview(expr->val.ident.name, expr->val.ident.len);
      break;
  }
  fputc(')', stdout);
}
#endif // _DEBUG

