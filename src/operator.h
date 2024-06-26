#ifndef _ZNC_OPERATOR_H
#define _ZNC_OPERATOR_H
#include "types.h"

// this value should reflect the highest return
// value of getprec() function. update this if
// necessary
#define OP_HIGHEST_PREC 10

typedef enum {
  OP_UNK = 0,
  OP_TRP_DOT,     // ...
  OP_DBL_AMP_EQL, // &&=
  OP_DBL_BAR_EQL, // ||=
  OP_DBL_LES_EQL, // <<=
  OP_DBL_GRT_EQL, // >>=
  OP_DBL_AMP,     // &&
  OP_DBL_BAR,     // ||
  OP_DBL_LES,     // <<
  OP_DBL_GRT,     // >>
  OP_DBL_PLS,     // ++   (unary)
  OP_DBL_DSH,     // --   (unary)
  OP_DBL_AST,     // **
  OP_DBL_EQL,     // ==
  OP_EXC_EQL,     // !=
  OP_GRT_EQL,     // >=
  OP_LES_EQL,     // <=
  OP_PLS_EQL,     // +=
  OP_DSH_EQL,     // -=
  OP_AST_EQL,     // *=
  OP_SLH_EQL,     // /=
  OP_PCT_EQL,     // %=
  OP_AMP_EQL,     // &=
  OP_BAR_EQL,     // |=
  OP_CRT_EQL,     // ^=
  OP_EXC,         // !    (unary)
  OP_PLS,         // +    (unary & binary)
  OP_DSH,         // -    (unary & binary)
  OP_AST,         // *
  OP_SLH,         // /
  OP_PCT,         // %
  OP_AMP,         // &
  OP_BAR,         // |
  OP_CRT,         // ^
  OP_TDL,         // ~    (unary)
  OP_QST,         // ?    (ternary)
  OP_CLN,         // :    (ternary)
  OP_CMM,         // ,
  OP_EQL,         // =
  OP_LES,         // <
  OP_GRT,         // >
  OP_DOT,         // .
  // the subscript operator is handled differently
  OP_SBC,         // [
} OperatorType;

// operators
extern const char *OperatorNames[];

/* returns the length of operator, 0 if op does not exist */
uvar isop(char *text);

/* returns OperatorType from given string */
OperatorType getop(char *text);

/* get operator precedence of binary operators */
int getprec(OperatorType type);

/* returns true if the given is a unary prefix operator */
int op_isprefix(OperatorType type);

/* returns true if the given is a unary postfix operator */
int op_ispostfix(OperatorType type);

/* returns true if the given is a binary operator */
int op_isinfix(OperatorType type);

// - ternary operators = lowest precedence
// - binary operators  = varied precedence
// - unary operators   = highest precedence

#endif // _ZNC_OPERATOR_H

