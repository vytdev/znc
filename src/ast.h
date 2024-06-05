#ifndef _ZNC_AST_H
#define _ZNC_AST_H
#include "types.h"
#include "lexer.h"
#include "arena.h"
#include "operator.h"
#include <stdbool.h>

typedef struct ASTIdentifier {
  char *name;           /* view to the name */
  uvar len;             /* length of the text */
} ASTIdentifier;

typedef struct ASTString {
  char *raw;
  uvar len;
} ASTString;

typedef struct ASTArray {
  struct ASTExpr *expr;
  struct ASTArray *next;
} ASTArray;

typedef struct ASTInteger {
  char *text;
  uvar len;
} ASTInteger;

typedef struct ASTUnaryOp {
  struct ASTExpr *val;
  bool isprefix;
  OperatorType op;
} ASTUnaryOp;

typedef struct ASTBinaryOp {
  struct ASTExpr *lhs;
  struct ASTExpr *rhs;
  OperatorType op;
} ASTBinaryOp;

typedef struct ASTTernaryOp {
  struct ASTExpr *lch;
  struct ASTExpr *mch;
  struct ASTExpr *rch;
  OperatorType op;
} ASTTernaryOp;

typedef enum {
  AST_EXPR_IDENTIFIER,
  AST_EXPR_STRING,
  AST_EXPR_ARRAY,
  AST_EXPR_INTEGER,
  AST_EXPR_UNOP,
  AST_EXPR_BINOP,
  AST_EXPR_TERNOP,
} ASTExprType;

typedef union {
  ASTIdentifier ident;
  ASTString str;
  ASTArray *arr;
  ASTInteger intg;
  ASTUnaryOp unop;
  ASTBinaryOp binop;
  ASTTernaryOp ternop;
} ASTExprVal;

typedef struct ASTExpr {
  ASTExprType type;
  ASTExprVal  val;
} ASTExpr;

typedef enum {
  AST_STM_EXPR,
} ASTStmType;

typedef union {
  ASTExpr *expr;
} ASTStmVal;

typedef struct ASTStm {
  ASTStmType type;
  ASTStmVal  val;
} ASTStm;

/* process identifiers */
ASTExpr *parse_identifier(Lexer *lex, Arena *arena);

/* parse expressions */
ASTExpr *parse_expr(Lexer *lex, Arena *arena);

/* process infix (binary) operators */
ASTExpr *parse_infix(Lexer *lex, Arena *arena, ASTExpr *lhs, int minprec);

/* process factor (unary and ternary ops) expressions */
ASTExpr *parse_factor(Lexer *lex, Arena *arena);

/* process primary (identifiers, literals, etc.) expressions */
ASTExpr *parse_primary(Lexer *lex, Arena *arena);

/* process secondary (member-access) expressions */
ASTExpr *parse_secondary(Lexer *lex, Arena *arena, ASTExpr *lhs);

/* process statements */
ASTStm *parse_statement(Lexer *lex, Arena *arena);

#ifdef _DEBUG

/* print expr */
void print_expr(ASTExpr *expr);

#endif // _DEBUG

#endif // _ZNC_AST_H

