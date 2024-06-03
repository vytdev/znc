#ifndef _ZNC_AST_H
#define _ZNC_AST_H
#include "types.h"
#include "lexer.h"
#include "arena.h"
#include "operator.h"

typedef struct ASTIdentifier {
  char *name;           /* view to the name */
  uvar len;             /* length of the text */
} ASTIdentifier;

typedef struct {
  struct ASTExpr *lhs;
  struct ASTExpr *rhs;
  OperatorType op;
} ASTBinaryOp;

typedef enum {
  AST_EXPR_IDENTIFIER,
  AST_EXPR_BINOP,
} ASTExprType;

typedef union {
  ASTIdentifier ident;
  ASTBinaryOp binop;
} ASTExprVal;

typedef struct ASTExpr {
  ASTExprType type;
  ASTExprVal  val;
} ASTExpr;

/* process identifiers */
ASTExpr *parse_identifier(Lexer *lex, Arena *arena);

/* parse expressions */
ASTExpr *parse_expr(Lexer *lex, Arena *arena);

/* process infix (binary) operators */
ASTExpr *parse_infix(Lexer *lex, Arena *arena, ASTExpr *lhs, int minprec);

/* process primary expressions */
ASTExpr *parse_factor(Lexer *lex, Arena *arena);

#ifdef _DEBUG

/* print expr */
void print_expr(ASTExpr *expr);

#endif // _DEBUG

#endif // _ZNC_AST_H

