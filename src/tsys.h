#ifndef _ZNC_TSYS_H
#define _ZNC_TSYS_H
#include "keyword.h"
#include "types.h"
#include "ast.h"
#include <stdbool.h>

typedef enum {
  PRIM_BYTE,
  PRIM_SHORT,
  PRIM_INT,
  PRIM_LONG,
  PRIM_UBYTE,
  PRIM_USHORT,
  PRIM_UINT,
  PRIM_ULONG,
  PRIM_FLOAT,
  PRIM_DOUBLE,
  PRIM_CHAR,
  PRIM_BOOL,
} PrimitiveType;

typedef struct TypeFuncArg {
  struct TypeFuncArg    *next;
  struct TypeSig        *arg;
  char                  *name;
  uvar                  len;
  ASTExpr               *def;
  bool                  rest;
} TypeFuncArg;

typedef struct TypeFunc {
  struct TypeSig        *ret;
  TypeFuncArg           *arg;
} TypeFunc;

typedef struct TypeName {
  char                  *name;
  uvar                  nlen;
} TypeName;

typedef enum {
  TYPE_PRIMITIVE,
  TYPE_FUNCTION,
  TYPE_NAME,
  TYPE_ARRAY,
} TypeClass;

typedef union {
  PrimitiveType         prim;
  TypeFunc              fn;
  TypeName              name;
  struct TypeSig        *array;
} TypeInfo;

typedef struct TypeSig {
  TypeClass             type;
  TypeInfo              info;
} TypeSig;

// primitive data type names
extern const char *PrimitiveTypeNames[];

/* returns primitive type from keyword */
PrimitiveType kwdtoprim(KeywordType kwd);

/* create a type signature structure from ASTTypeRef */
TypeSig *typesig_fromast(ASTTypeRef *node);

/* free a type signature structure */
void typesig_free(TypeSig *sig);

#endif // _ZNC_TSYS_H

