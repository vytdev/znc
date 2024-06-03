#include "test.h"
#include "../src/lexer.h"

int test_tokenizer(void) {
  Lexer lex;
  lexer_init(&lex, "<test_tokenizer>",
      "/* this should be\n"
      "ignored */\n"
      "// this too\n"
  );

  // get next token
  Token *tok = lexer_consume(&lex);
  if (!EXPECT_NE(tok, NULL)) {
    lexer_free(&lex);
    return 1;
  }
  print_token(tok, "a token\n");

  // expect eof
  tok = lexer_peek(&lex, 0);
  if (expect_token(tok, TOKEN_EOF, NULL)) {
    lexer_free(&lex);
    return 1;
  }

  lexer_free(&lex);
  return 0;
}

int test(const char *name) {
  TEST_REGISTER(test_tokenizer);
  TEST_RUN(test_tokenizer);
  return 0;
}

