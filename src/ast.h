#ifndef _ZNC_AST_H
#define _ZNC_AST_H
#include "types.h"
#include "lexer.h"
#include "arena.h"

typedef struct {
  char *name;           /* view to the name */
  uvar len;             /* length of the text */
} ASTIdentifier;

/* process identifiers */
ASTIdentifier *ast_identifier(Lexer *lex, Arena *arena);

#endif // _ZNC_AST_H

