#ifndef _ZNC_AST_H
#define _ZNC_AST_H
#include "keyword.h"
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

typedef struct ASTFuncArg {
  struct ASTFuncArg *next;
  char *target;
  uvar tlen;
  struct ASTExpr *val;
} ASTFuncArg;

typedef struct ASTFuncCall {
  struct ASTExpr *fname;
  ASTFuncArg *args;
} ASTFuncCall;

typedef struct ASTTypeCast {
  struct ASTExpr *val;
  struct ASTTypeRef *type;
} ASTTypeCast;

typedef enum {
  AST_EXPR_IDENTIFIER,
  AST_EXPR_STRING,
  AST_EXPR_ARRAY,
  AST_EXPR_INTEGER,
  AST_EXPR_UNOP,
  AST_EXPR_BINOP,
  AST_EXPR_TERNOP,
  AST_EXPR_CALL,
  AST_EXPR_CAST,
} ASTExprType;

typedef union {
  ASTIdentifier ident;
  ASTString str;
  ASTArray *arr;
  ASTInteger intg;
  ASTUnaryOp unop;
  ASTBinaryOp binop;
  ASTTernaryOp ternop;
  ASTFuncCall fcall;
  ASTTypeCast cast;
} ASTExprVal;

typedef struct ASTExpr {
  Token *tok;
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
  AST_STM_RETURN,
  AST_STM_BLOCK,
} ASTStmType;

typedef union {
  ASTExpr *expr;
  ASTLet let;
  ASTIfElse ifels;
  ASTWhile whil;
  ASTExpr *retval;
  struct ASTBlock *blck;
} ASTStmVal;

typedef struct ASTStm {
  Token *tok;
  struct ASTStm *next; // used on ast blocks
  ASTStmType type;
  ASTStmVal  val;
} ASTStm;

typedef struct ASTBlock {
  ASTStm *head;
  ASTStm *tail;
} ASTBlock;

typedef struct ASTFuncArgDef {
  Token *tok;
  struct ASTFuncArgDef *next;
  struct ASTTypeRef *type;
  char *name;
  uvar nlen;
  ASTExpr *defval;
  bool restarr;         // ...
} ASTFuncArgDef;

typedef struct ASTFuncDef {
  Token *tok;
  char *name;
  uvar nlen;
  ASTFuncArgDef *args;
  struct ASTTypeRef *rettype;
  ASTBlock *code;
} ASTFuncDef;

typedef struct ASTEnumEntry {
  Token *tok;
  struct ASTEnumEntry *next;
  char *name;
  uvar nlen;
  ASTExpr *cnst;
} ASTEnumEntry;

typedef struct ASTEnum {
  Token *tok;
  char *name;
  uvar nlen;
  ASTEnumEntry *head;
  ASTEnumEntry *tail;
  struct ASTTypeRef *type;
} ASTEnum;

typedef struct ASTTypeAlias {
  Token *tok;
  char *name;
  uvar nlen;
  struct ASTTypeRef *type;
} ASTTypeAlias;

typedef struct ASTFuncType {
  ASTFuncArgDef *args;
  struct ASTTypeRef *ret;
} ASTFuncType;

typedef struct ASTTypeName {
  char *name;
  uvar nlen;
} ASTTypeName;

typedef enum {
  AST_TYPE_PRIMITIVE,
  AST_TYPE_ARRAY,
  AST_TYPE_FUNCTION,
  AST_TYPE_NAME,
} ASTTypeType;

typedef union {
  KeywordType type;
  struct ASTTypeRef *aelem;
  ASTFuncType func;
  ASTTypeName tname;
} ASTTypeVal;

typedef struct ASTTypeRef {
  Token *tok;
  ASTTypeType type;
  ASTTypeVal  val;
} ASTTypeRef;

typedef enum {
  AST_ROOT_FUNCDEF,
  AST_ROOT_ENUM,
  AST_ROOT_TALIAS,
} ASTDeclType;

typedef union {
  ASTFuncDef *func;
  ASTEnum *enumr;
  ASTTypeAlias *talias;
} ASTDeclVal;

typedef struct ASTDecl {
  struct ASTDecl *next;
  ASTDeclType type;
  ASTDeclVal  val;
  // TODO: imports and exports
} ASTDecl;

typedef struct ASTRoot {
  ASTDecl *head;
  ASTDecl *tail;
} ASTRoot;

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

/* process function definitions */
ASTFuncDef *parse_funcdef(Lexer *lex, Arena *arena);

/* process function args (for definitions) */
ASTFuncArgDef *parse_funcarg(Lexer *lex, Arena *arena, bool *err);

/* process enum definitions */
ASTEnum *parse_enum(Lexer *lex, Arena *arena);

/* process type references */
ASTTypeRef *parse_typeref(Lexer *lex, Arena *arena);

/* process type aliases */
ASTTypeAlias *parse_typealias(Lexer *lex, Arena *arena);

/* process root node */
ASTRoot *parse_root(Lexer *lex, Arena *arena);

/* parse ast tree given the source lexer and an arena allocator */
ASTRoot *parse(Lexer *lex, Arena *arena);

#ifdef _DEBUG

/* print expr */
void print_expr(ASTExpr *expr);

#endif // _DEBUG

#endif // _ZNC_AST_H

