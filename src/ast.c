#include "ast.h"
#include "lexer.h"
#include "arena.h"
#include "token.h"
#include "operator.h"
#include "keyword.h"
#include <stdio.h>
#include <stdbool.h>
#ifdef _DEBUG
#include "util.h"
#endif // _DEBUG

// NOTES:
// - it is not necessary to check the 'lex' and 'arena' ptr

ASTExpr *parse_identifier(Lexer *lex, Arena *arena) {
  ASTExpr *node = aaloc(arena, ASTExpr);
  if (!node) return NULL;

  // consume token, check if this is an identifier
  Token *tok = lexer_consume(lex);
  if (!tok)
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
  if (!lhs) return NULL;

  // get next token
  Token *next = lexer_peek(lex, 1);
  if (!next) return NULL;

  // while token
  while (next->type == TOKEN_OPERATOR && getprec(getop(next->lexeme)) >= minprec) {
    lexer_consume(lex);
    OperatorType op = getop(next->lexeme);

    // process right hand side operand
    ASTExpr *rhs = parse_factor(lex, arena);
    if (!rhs) return NULL;

    // process higher-precedence tokens
    next = lexer_peek(lex, 1);
    if (!next) return NULL;
    while (next->type == TOKEN_OPERATOR && getprec(getop(next->lexeme)) > minprec) {
      rhs = parse_infix(lex, arena, rhs, minprec + 1);
      next = lexer_peek(lex, 1);
     if (!next) return NULL;
    }

    // new expr
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_BINOP;
    node->val.binop.lhs = lhs;
    node->val.binop.op = op;
    node->val.binop.rhs = rhs;
    lhs = node;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
 }

  return lhs;
}

ASTExpr *parse_factor(Lexer *lex, Arena *arena) {
  Token *next = lexer_peek(lex, 1);
  if (!next)
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
    if (!expr) return NULL;
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
  if (!next) return NULL;

  // while there's still postfix ops...
  while (next->type == TOKEN_OPERATOR && op_ispostfix(getop(next->lexeme))) {
    lexer_consume(lex);

    // new node
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_UNOP;
    node->val.unop.op = getop(next->lexeme);
    node->val.unop.isprefix = false;
    node->val.unop.val = expr;
    expr = node;

    // for possible subsequent postfix ops
    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  // check ternary
  if (cmp_token(next, TOKEN_OPERATOR, "?")) {
    lexer_consume(lex); // consume '?'

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
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
    if (!next) return NULL;
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
  if (!next)
    return NULL;

  // identifier token
  if (next->type == TOKEN_IDENTIFIER)
    return parse_secondary(lex, arena, parse_identifier(lex, arena));

  // string token
  if (next->type == TOKEN_STRING) {
    lexer_consume(lex);
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_STRING;
    node->val.str.raw = next->lexeme;
    node->val.str.len = next->len;
    return parse_secondary(lex, arena, node);
  }

  // array literals
  if (cmp_token(next, TOKEN_BRACKET, "[")) {
    lexer_consume(lex); // consume '['
    ASTExpr *expr = aaloc(arena, ASTExpr);
    if (!expr) return NULL;
    expr->type = AST_EXPR_ARRAY;
    ASTArray *arr = NULL;

    while (1) {
      next = lexer_peek(lex, 1);
      if (!next) return NULL;

      // maybe end?
      if (cmp_token(next, TOKEN_BRACKET, "]"))
        break;

      // create a new element
      ASTArray *newarr = aaloc(arena, ASTArray);
      if (!newarr) return NULL;
      if (arr) arr->next = newarr;
      arr = newarr;
      arr->expr = NULL;
      arr->next = NULL;
      if (!expr->val.arr) expr->val.arr = arr; // first element

      // process expression (minimum precedence 2 to skip comma)
      ASTExpr *node = parse_infix(lex, arena, parse_factor(lex, arena), 2);
      if (!node) return NULL;
      arr->expr = node;

      // expect ']' or ','
      next = lexer_peek(lex, 1);
      if (!next) return NULL;
      if (
        !cmp_token(next, TOKEN_BRACKET, "]") &&
        !cmp_token(next, TOKEN_OPERATOR, ",")
      ) {
        print_token(next, "syntax error: expected either of ']' and ','\n");
        return NULL;
      }

      // consume if this is a comma
      if (cmp_token(next, TOKEN_OPERATOR, ","))
        lexer_consume(lex);
    }

    // end ']'
    next = lexer_consume(lex);
    if (expect_token(next, TOKEN_BRACKET, "]")) return NULL;
    return parse_secondary(lex, arena, expr);
  }

  // integers
  if (cmp_token(next, TOKEN_INTEGER, NULL)) {
    lexer_consume(lex); // consume the int
    ASTExpr *val = aaloc(arena, ASTExpr);
    if (!val) return NULL;
    val->type = AST_EXPR_INTEGER;
    val->val.intg.text = next->lexeme;
    val->val.intg.len = next->len;
    return parse_secondary(lex, arena, val);
  }

  // expression enclosed with paren
  if (cmp_token(next, TOKEN_BRACKET, "(")) {
    lexer_consume(lex);
    ASTExpr *expr = parse_expr(lex, arena);
    if (!expr) return NULL;
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_BRACKET, ")")) return NULL;
    return parse_secondary(lex, arena, expr);
  }

  expect_token(next, -1, NULL);
  return NULL;
}

ASTExpr *parse_secondary(Lexer *lex, Arena *arena, ASTExpr *lhs) {
  if (!lhs) return NULL;

  Token *next = lexer_peek(lex, 1);
  if (!next)
    return NULL;

  // check whether this is a member-access or subscript op
  if (
    !cmp_token(next, TOKEN_OPERATOR, ".") &&
    !cmp_token(next, TOKEN_BRACKET, "[")
  ) return lhs;

  // check member-access
  while (cmp_token(next, TOKEN_OPERATOR, ".")) {
    lexer_consume(lex); // consume '.'

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_BINOP;
    node->val.binop.op = OP_DOT;
    node->val.binop.lhs = lhs;

    // the member name
    ASTExpr *memb = parse_identifier(lex, arena);
    if (!memb) return NULL;
    node->val.binop.rhs = memb;

    lhs = node;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  // check subscript
  while (cmp_token(next, TOKEN_BRACKET, "[")) {
    lexer_consume(lex); // consume '['

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_BINOP;
    node->val.binop.op = OP_SBC;
    node->val.binop.lhs = lhs;

    // the key
    ASTExpr *sub = parse_expr(lex, arena);
    if (!sub) return NULL;
    node->val.binop.rhs = sub;

    // consume ']'
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_BRACKET, "]"))
      return NULL;

    lhs = node;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  return parse_secondary(lex, arena, lhs);
}

ASTStm *parse_statement(Lexer *lex, Arena *arena) {
  Token *next = lexer_peek(lex, 1);
  if (!next) return NULL;
  ASTStm *stm = aaloc(arena, ASTStm);
  if (!stm) return NULL;

  if (next->type == TOKEN_KEYWORD) {
    lexer_consume(lex); // consume the keyword
    KeywordType kwd = getkwd(next->lexeme);

    switch (kwd) {
      case KWD_LET: {
        stm->type = AST_STM_LET;
        // TODO: integrate types

        // get identifier
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_IDENTIFIER, NULL))
          return NULL;
        stm->val.let.name = next->lexeme;
        stm->val.let.nlen = next->len;

        // check whether there is initial value
        next = lexer_consume(lex);
        if (!next) return NULL;
        stm->val.let.initval = NULL;

        if (cmp_token(next, TOKEN_OPERATOR, "=")) {
          ASTExpr *initval = parse_expr(lex, arena);
          if (!initval) return NULL;
          stm->val.let.initval = initval;
          next = lexer_consume(lex);
          if (!next) return NULL;
        }

        // expect semi-colon
        if (expect_token(next, TOKEN_DELIMETER, ";"))
          return NULL;
        return stm;
      }

      case KWD_IF: {
        stm->type = AST_STM_IFELSE;

        // expect condition opening '('
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, "("))
          return NULL;

        ASTExpr *cond = parse_expr(lex, arena);
        if (!cond) return NULL;
        stm->val.ifels.cond = cond;

        // expect condition closing ')'
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, ")"))
          return NULL;

        // now the code to execute
        ASTStm *code = parse_statement(lex, arena);
        if (!code) return NULL;
        stm->val.ifels.code = code;

        // the else statement
        stm->val.ifels.elsec = NULL;
        next = lexer_peek(lex, 1);
        if (!next) return NULL;
        if (cmp_token(next, TOKEN_KEYWORD, "else")) {
          lexer_consume(lex); // consume 'else'
          ASTStm *elsec = parse_statement(lex, arena);
          if (!elsec) return NULL;
          stm->val.ifels.elsec = elsec;
        }

        return stm;
      }

      case KWD_WHILE: {
        stm->type = AST_STM_WHILE;

        // expect condition opening '('
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, "("))
          return NULL;

        ASTExpr *cond = parse_expr(lex, arena);
        if (!cond) return NULL;
        stm->val.whil.cond = cond;

        // expect condition closing ')'
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, ")"))
          return NULL;

        // code to execute
        ASTStm *code = parse_statement(lex, arena);
        if (!code) return NULL;
        stm->val.whil.code = code;

        return stm;
      }

      // it is impossible to get unknown keyword, but just in-case :)
      default:
        print_token(next, "parse error: unknown keyword\n");
        return NULL;
    }
  }

  // TODO: process other statements here

  // expressions
  stm->type = AST_STM_EXPR;
  ASTExpr *expr = parse_expr(lex, arena);
  if (!expr) return NULL;
  stm->val.expr = expr;

  // semi-colon
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_DELIMETER, ";"))
    return NULL;

  return stm;
}

#ifdef _DEBUG
void print_expr(ASTExpr *expr) {
  if (!expr) {
    printf("[null]");
    return;
  }

  switch (expr->type) {
    case AST_EXPR_UNOP:
      if (expr->val.unop.isprefix) printf("%s", OperatorNames[expr->val.unop.op]);
      fputc('(', stdout);
      print_expr(expr->val.unop.val);
      fputc(')', stdout);
      if (!expr->val.unop.isprefix) printf("%s", OperatorNames[expr->val.unop.op]);
      break;
    case AST_EXPR_BINOP:
      fputc('(', stdout);
      print_expr(expr->val.binop.lhs);
      if (expr->val.binop.op == OP_SBC) {
        fputc('[', stdout);
        print_expr(expr->val.binop.rhs);
        fputc(']', stdout);
      } else {
        printf(" %s ", OperatorNames[expr->val.binop.op]);
        print_expr(expr->val.binop.rhs);
      }
      fputc(')', stdout);
      break;
    case AST_EXPR_TERNOP:
      fputc('(', stdout);
      if (expr->val.ternop.op != OP_QST) {
        printf("null)");
        return;
      }
      print_expr(expr->val.ternop.lch);
      printf(" ? ");
      print_expr(expr->val.ternop.mch);
      printf(" : ");
      print_expr(expr->val.ternop.rch);
      fputc(')', stdout);
      break;
    case AST_EXPR_IDENTIFIER:
      pview(expr->val.ident.name, expr->val.ident.len);
      break;
  }
}
#endif // _DEBUG

