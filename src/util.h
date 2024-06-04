#ifndef _ZNC_UTIL_H
#define _ZNC_UTIL_H
#include "types.h"

/* read a text file */
char *util_readfile(const char *path);

/* print string up to a given length */
void pview(char *str, uvar len);

#endif // _ZNC_UTIL_H

