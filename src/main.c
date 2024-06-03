#include "token.h"
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
  Arena arena;
  if (!arena_init(&arena)) {
    fprintf(stderr, "znc: failed to init arena\n");
    return 1;
  }

  // TODO: ast gen
  while (1) {
    // just to break the loop
    Token *next = lexer_peek(&lex, 1);
    if (!next || next->type != TOKEN_IDENTIFIER)
      break;

    // print identifier nodes
    ASTIdentifier *node = ast_identifier(&lex, &arena);
    if (!node) break;
    pview(node->name, node->len);
    fputc('\n', stdout);
  }

  arena_free(&arena);
  lexer_free(&lex);
  return 1;
}

