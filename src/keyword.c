#include "keyword.h"
#include "types.h"
#include <string.h>

const char *KeywordNames[] = {
  // should be ordered from longest string to shortest
  [KWD_UNK]             = "<unknown>",

  "else",
  "let",
  "if",
};

uvar iskwd(char *text) {
  for (int i = 1; i < sizeof(KeywordNames) / sizeof(char*); i++) {
    uvar len = strlen(KeywordNames[i]);
    if (strncmp(text, KeywordNames[i], len) == 0)
      return len;
  }
  return 0;
}

KeywordType getkwd(char *text) {
  for (int i = 1; i < sizeof(KeywordNames) / sizeof(char*); i++)
    if (strncmp(text, KeywordNames[i], strlen(KeywordNames[i])) == 0)
      return i;
  return KWD_UNK;
}

