#include "ast.h"
#include "lexer.h"
#include "arena.h"
#include "token.h"
#include "operator.h"
#include "util.h"
#include <stdio.h>
#include <stdbool.h>

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

    next = lexer_peek(lex, 1);
    if (valptr(next)) return NULL;
 }

  return lhs;
}

ASTExpr *parse_factor(Lexer *lex, Arena *arena) {
  Token *next = lexer_peek(lex, 1);
  if (valptr(next))
    return NULL;

  // hierarchy:
  //   a b C d e   ->   (a(b(((C)d)e)))
  //   d -> e -> b -> a
  // unary precedence rules apply:
  //   right -> right-most -> left -> left-most

  // prefix unary operators
  if (next->type == TOKEN_OPERATOR && op_isprefix(getop(next->lexeme))) {
    lexer_consume(lex);

    // make a new node containing the unary op
    ASTExpr *expr = aaloc(arena, ASTExpr);
    if (valptr(expr)) return NULL;
    expr->type = AST_EXPR_UNOP;
    expr->val.unop.op = getop(next->lexeme);
    expr->val.unop.isprefix = true;

    // process node value
    ASTExpr *sub = parse_factor(lex, arena);
    if (!sub) return NULL;
    expr->val.unop.val = sub;

    return expr;
  }

  // process primary
  ASTExpr *expr = parse_primary(lex, arena);
  if (!expr) return NULL;
  next = lexer_peek(lex, 1);
  if (valptr(next)) return NULL;

  // while there's still postfix ops...
  while (next->type == TOKEN_OPERATOR && op_ispostfix(getop(next->lexeme))) {
    lexer_consume(lex);

    // new node
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (valptr(node)) return NULL;
    node->type = AST_EXPR_UNOP;
    node->val.unop.op = getop(next->lexeme);
    node->val.unop.isprefix = false;
    node->val.unop.val = expr;
    expr = node;

    // for possible subsequent postfix ops
    next = lexer_peek(lex, 1);
    if (valptr(next)) return NULL;
  }

  // check ternary
  next = lexer_peek(lex, 1);
  if (valptr(next)) return NULL;
  if (next->type == TOKEN_OPERATOR && getop(next->lexeme) == OP_QST) {
    lexer_consume(lex); // consume '?'

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (valptr(node)) return NULL;
    node->type = AST_EXPR_TERNOP;
    node->val.ternop.op = OP_QST;
    node->val.ternop.lch = expr;

    ASTExpr *sub = NULL;

    // true expression
    sub = parse_factor(lex, arena);
    if (!sub) return NULL;
    node->val.ternop.mch = sub;

    // consume colon
    next = lexer_consume(lex);
    if (expect_token(next, TOKEN_OPERATOR, ":"))
      return NULL;

    // false expression
    sub = parse_factor(lex, arena);
    if (!sub) return NULL;
    node->val.ternop.rch = sub;

    expr = node;
  }

  return expr;
}

ASTExpr *parse_primary(Lexer *lex, Arena *arena) {
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
    case AST_EXPR_UNOP:
      if (expr->val.unop.isprefix) printf("%s", OperatorNames[expr->val.unop.op]);
      print_expr(expr->val.unop.val);
      if (!expr->val.unop.isprefix) printf("%s", OperatorNames[expr->val.unop.op]);
      break;
    case AST_EXPR_BINOP:
      print_expr(expr->val.binop.lhs);
      printf(" %s ", OperatorNames[expr->val.binop.op]);
      print_expr(expr->val.binop.rhs);
      break;
    case AST_EXPR_TERNOP:
      if (expr->val.ternop.op != OP_QST) {
        printf("null)");
        return;
      }
      print_expr(expr->val.ternop.lch);
      printf(" ? ");
      print_expr(expr->val.ternop.mch);
      printf(" : ");
      print_expr(expr->val.ternop.rch);
      break;
    case AST_EXPR_IDENTIFIER:
      pview(expr->val.ident.name, expr->val.ident.len);
      break;
  }
  fputc(')', stdout);
}
#endif // _DEBUG

