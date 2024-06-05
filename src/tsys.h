#ifndef _ZNC_TSYS_H
#define _ZNC_TSYS_H
#include "keyword.h"

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

// primitive data type names
extern const char *PrimitiveTypeNames[];

/* returns primitive type from keyword */
PrimitiveType kwdtoprim(KeywordType kwd);

#endif // _ZNC_TSYS_H

