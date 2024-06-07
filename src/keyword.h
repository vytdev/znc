#ifndef _ZNC_KEYWORD_H
#define _ZNC_KEYWORD_H
#include "types.h"

typedef enum {
  KWD_UNK = 0,

  KWD_LET,
  KWD_IF,
  KWD_ELSE,
  KWD_WHILE,
  KWD_RETURN,

  KWD_FUNCTION,
  KWD_ENUM,
  KWD_TYPE,

  KWD_BYTE,
  KWD_SHORT,
  KWD_INT,
  KWD_LONG,
  KWD_UBYTE,
  KWD_USHORT,
  KWD_UINT,
  KWD_ULONG,
  KWD_FLOAT,
  KWD_DOUBLE,
  KWD_CHAR,
  KWD_BOOL,
} KeywordType;

// keywords
extern const char *KeywordNames[];

/* returns the length of the keyword if it is on the KeywordNames, zero otheewise */
uvar iskwd(char *text);

/* get the keyword type from given string */
KeywordType getkwd(char *text);

/* returns whether the keyword is a primitive type */
int iskwdprim(KeywordType kwd);

#endif // _ZNC_KEYWORD_H

