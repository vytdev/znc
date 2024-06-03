#include "util.h"
#include "types.h"
#include <stdlib.h>
#include <stdio.h>

char *util_readfile(const char *path) {
  if (!path)
    return NULL;

  // try to open file
  FILE *fp = fopen(path, "r");
  if (!fp)
    return NULL;

  // get the size of the file
  fseek(fp, 0, SEEK_END);
  uvar size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  // allocate memory for string, +1 for the NUL-terminator
  char *text = (char*)malloc(size + 1);
  if (!text) {
    fclose(fp);
    return NULL;
  }
  text[size] = '\0';

  // read file
  fread(text, 1, size, fp);
  fclose(fp);
  return text;
}

void pview(char *str, uvar len) {
  fwrite(str, 1, len, stdout);
}

int valptr(void *ptr) {
  if (!ptr) {
    fprintf(stderr, "znc: out of memory\n");
    return 1;
  }
  return 0;
}

