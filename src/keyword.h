#ifndef _ZNC_KEYWORD_H
#define _ZNC_KEYWORD_H
#include "types.h"

typedef enum {
  KWD_UNK = 0,
  KWD_ELSE,
  KWD_LET,
  KWD_IF,
} KeywordType;

// keywords
extern const char *KeywordNames[];

/* returns the length of the keyword if it is on the KeywordNames, zero otheewise */
uvar iskwd(char *text);

/* get the keyword type from given string */
KeywordType getkwd(char *text);

#endif // _ZNC_KEYWORD_H

