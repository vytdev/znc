#ifndef _ZNC_AST_H
#define _ZNC_AST_H
#include "types.h"
#include "lexer.h"
#include "arena.h"
#include "operator.h"
#include "tsys.h"
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

typedef struct ASTLet {
  char *name;
  uvar nlen;
  ASTExpr *initval;
  struct ASTTypeRef *type;
} ASTLet;

typedef struct ASTIfElse {
  ASTExpr *cond;
  struct ASTStm *code;
  struct ASTStm *elsec;
} ASTIfElse;

typedef struct ASTWhile {
  ASTExpr *cond;
  struct ASTStm *code;
} ASTWhile;

typedef enum {
  AST_STM_EXPR,
  AST_STM_LET,
  AST_STM_IFELSE,
  AST_STM_WHILE,
  AST_STM_BLOCK,
} ASTStmType;

typedef union {
  ASTExpr *expr;
  ASTLet let;
  ASTIfElse ifels;
  ASTWhile whil;
  struct ASTBlock *blck;
} ASTStmVal;

typedef struct ASTStm {
  struct ASTStm *next; // used on ast blocks
  ASTStmType type;
  ASTStmVal  val;
} ASTStm;

typedef struct ASTBlock {
  ASTStm *head;
  ASTStm *tail;
} ASTBlock;

typedef enum {
  AST_TYPE_PRIMITIVE,
  AST_TYPE_ARRAY,
} ASTTypeType;

typedef union {
  PrimitiveType type;
  struct ASTTypeRef *aelem;
} ASTTypeVal;

typedef struct ASTTypeRef {
  ASTTypeType type;
  ASTTypeVal  val;
} ASTTypeRef;

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

/* process code blocks */
ASTBlock *parse_block(Lexer *lex, Arena *arena);

/* process type references */
ASTTypeRef *parse_typeref(Lexer *lex, Arena *arena);

#ifdef _DEBUG

/* print expr */
void print_expr(ASTExpr *expr);

#endif // _DEBUG

#endif // _ZNC_AST_H

