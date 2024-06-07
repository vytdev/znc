#include "tsys.h"
#include <stdlib.h>
#include <stdbool.h>

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

static TypeFuncArg *typesig_argast(ASTFuncArgDef *node, bool *err) {
  if (!node) return NULL;

  TypeFuncArg *arg = (TypeFuncArg*)malloc(sizeof(TypeFuncArg));
  if (!arg) {
    *err = true;
    return NULL;
  }

  TypeSig *type = typesig_fromast(node->type);
  if (!type) {
    *err = true;
    return NULL;
  }

  arg->arg  = type;
  arg->def  = node->defval;
  arg->name = node->name;
  arg->len  = node->nlen;
  arg->rest = node->restarr;
  arg->next = typesig_argast(node->next, err);

  return arg;
}

static void typesig_free_fnarg(TypeFuncArg *arg) {
  if (!arg) return;
  typesig_free_fnarg(arg->next);
  typesig_free(arg->arg);
}

TypeSig *typesig_fromast(ASTTypeRef *node) {
  if (!node) return NULL;

  TypeSig *sig = (TypeSig*)malloc(sizeof(TypeSig));
  if (!sig) return NULL;

  switch (node->type) {
    case AST_TYPE_PRIMITIVE:
      sig->type = TYPE_PRIMITIVE;
      sig->info.prim = kwdtoprim(node->val.type);
      break;
    case AST_TYPE_ARRAY:
      sig->type = TYPE_ARRAY;
      TypeSig *elem = typesig_fromast(node->val.aelem);
      if (!elem) return NULL;
      sig->info.array = elem;
      break;
    case AST_TYPE_FUNCTION:
      sig->type = TYPE_FUNCTION;
      TypeSig *ret = typesig_fromast(node->val.func.ret);
      if (!ret) return NULL;
      sig->info.fn.ret = ret;
      bool err = false;
      TypeFuncArg *arg = typesig_argast(node->val.func.args, &err);
      if (err) return NULL;
      sig->info.fn.arg = arg;
      break;
    case AST_TYPE_NAME:
      sig->type = TYPE_NAME;
      sig->info.name.name = node->val.tname.name;
      sig->info.name.nlen = node->val.tname.nlen;
      break;
  }

  return sig;
}

void typesig_free(TypeSig *sig) {
  if (!sig) return;

  switch (sig->type) {
    case TYPE_FUNCTION:
      typesig_free(sig->info.fn.ret);
      typesig_free_fnarg(sig->info.fn.arg);
      break;
    case TYPE_ARRAY:
      typesig_free(sig->info.array);
      break;
    default:
      break;
  }

  free(sig);
}

