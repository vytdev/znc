#include "tsys.h"

const char *PrimitiveTypeNames[] = {
  [PRIM_BYTE]   = "byte",
  [PRIM_SHORT]  = "short",
  [PRIM_INT]    = "int",
  [PRIM_LONG]   = "long",
  [PRIM_UBYTE]  = "ubyte",
  [PRIM_USHORT] = "ushort",
  [PRIM_UINT]   = "uint",
  [PRIM_ULONG]  = "ulong",
  [PRIM_FLOAT]  = "float",
  [PRIM_DOUBLE] = "double",
  [PRIM_CHAR]   = "char",
  [PRIM_BOOL]   = "bool",
};

PrimitiveType kwdtoprim(KeywordType kwd) {
  PrimitiveType type = kwd - KWD_BYTE;
  if (type < PRIM_BYTE || type > PRIM_BOOL) return -1;
  return type;
}

