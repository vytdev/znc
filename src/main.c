#include "util.h"
#include "lexer.h"
#include "ast.h"
#include "arena.h"
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

  // init arena
  Arena *arena = arena_init(ARENA_MINSIZE);
  if (!arena) {
    fprintf(stderr, "znc: failed to init arena\n");
    lexer_free(&lex);
    return 1;
  }

  // parse type
  ASTTypeRef *node = parse_typeref(&lex, arena);
  if (!node)
    fprintf(stderr, "znc: aborting due to error\n");

  arena_free(arena);
  lexer_free(&lex);
  return 1;
}

