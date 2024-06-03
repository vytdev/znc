#include "operator.h"
#include <string.h>

const char *OperatorNames[] = {
  // ordered from longest string to shortest
  [OP_UNK]      = "<unknown>",

  "&&=", "||=", "<<=", ">>=",

  "&&", "||", "<<", ">>", "++", "--",
  "**", "==", "!=", ">=", "<=", "+=",
  "-=", "*=", "/=", "%=", "&=", "|=",
  "^=",

  "!", "+", "-", "*", "/", "%",
  "&", "|", "^", "~", "?", ":",
  ",", "=", "<", ">", ".",
};

uvar isop(char *text) {
  for (int i = 1; i < sizeof(OperatorNames) / sizeof(char*); i++) {
    uvar len = strlen(OperatorNames[i]);
    if (strncmp(text, OperatorNames[i], len) == 0)
      return len;
  }
  return 0;
}

OperatorType getop(char *text) {
  for (int i = 1; i < sizeof(OperatorNames) / sizeof(char*); i++) {
    if (strncmp(text, OperatorNames[i], strlen(OperatorNames[i])) == 0)
      return i;
  }
  return OP_UNK;
}

int getprec(OperatorType type) {
  switch (type) {
    // comma
    case OP_CMM:
      return 1;

    // assignment
    case OP_EQL:
    case OP_DBL_AMP_EQL:
    case OP_DBL_BAR_EQL:
    case OP_DBL_LES_EQL:
    case OP_DBL_GRT_EQL:
    case OP_PLS_EQL:
    case OP_DSH_EQL:
    case OP_AST_EQL:
    case OP_SLH_EQL:
    case OP_PCT_EQL:
    case OP_AMP_EQL:
    case OP_BAR_EQL:
    case OP_CRT_EQL:
      return 2;

    // logical OR
    case OP_DBL_BAR:
      return 3;

    // logical AND
    case OP_DBL_AMP:
      return 4;

    // comparison
    case OP_DBL_EQL:
    case OP_EXC_EQL:
    case OP_GRT:
    case OP_GRT_EQL:
    case OP_LES:
    case OP_LES_EQL:
      return 5;

    // bitwise AND, OR, and XOR
    case OP_AMP:
    case OP_BAR:
    case OP_CRT:
      return 6;

    // bitwise SHIFT LEFT and SHIFT RIGHT
    case OP_DBL_LES:
    case OP_DBL_GRT:
      return 7;

    // arithmetic ADD and SUBTRACT
    case OP_PLS:
    case OP_DSH:
      return 8;

    // arithmetic MULTIPLY, DIVIDE, and MODULO
    case OP_AST:
    case OP_SLH:
    case OP_PCT:
      return 9;

    // arithmetic EXPONENT
    case OP_DBL_AST:
      return 10;

    // member-access operator
    case OP_DOT:
      return 11;

    // default case
    default:
      return 0;
  }
}

int op_isprefix(OperatorType type) {
  switch (type) {
    case OP_DBL_PLS:
    case OP_DBL_DSH:
    case OP_EXC:
    case OP_PLS:
    case OP_DSH:
    case OP_TDL:
      return 1;
    default:
      return 0;
  }
}

int op_ispostfix(OperatorType type) {
  switch (type) {
    case OP_DBL_PLS:
    case OP_DBL_DSH:
      return 1;
    default:
      return 0;
  }
}

int op_isinfix(OperatorType type) {
  return getprec(type) > 0;
}

