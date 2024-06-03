#include "util.h"
#include "lexer.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "znc: too few arguments\n");
    return 1;
  }

  // read the file
  char *text = util_readfile(argv[1]);
  if (!text) {
    fprintf(stderr, "znc: failed to read file: %s\n", argv[1]);
    return 1;
  }

  // init lexer
  Lexer lex;
  if (lexer_init(&lex, argv[1], text)) {
    fprintf(stderr, "znc: failed to init lexer\n");
    return 1;
  }

  // TODO: ast gen
  while (!lex.eof) {
    Token *tok = lexer_consume(&lex);
    if (!tok) break;
    print_token(tok, "token type=%s\n", TokenTypeNames[tok->type]);
  }

  lexer_free(&lex);
  return 1;
}

