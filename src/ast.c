#include "ast.h"
#include "lexer.h"
#include "arena.h"
#include "token.h"
#include "operator.h"
#include "keyword.h"
#include "tsys.h"
#include <stdio.h>
#include <stdbool.h>
#ifdef _DEBUG
#include "util.h"
#endif // _DEBUG

// NOTES:
// - it is not necessary to check the 'lex' and 'arena' ptr

ASTExpr *parse_identifier(Lexer *lex, Arena *arena) {
  ASTExpr *node = aaloc(arena, ASTExpr);
  if (!node) return NULL;

  // consume token, check if this is an identifier
  Token *tok = lexer_consume(lex);
  if (!tok)
    return NULL;

  if (expect_token(tok, TOKEN_IDENTIFIER, NULL))
    return NULL;

  // set some fields
  node->type = AST_EXPR_IDENTIFIER;
  node->val.ident.name = tok->lexeme;
  node->val.ident.len = tok->len;

  return node;
}

ASTExpr *parse_expr(Lexer *lex, Arena *arena) {
  return parse_infix(lex, arena, parse_factor(lex, arena), 1);
}

ASTExpr *parse_infix(Lexer *lex, Arena *arena, ASTExpr *lhs, int minprec) {
  if (!lhs) return NULL;

  // ensure the left hand side is processed
  if (minprec > OP_HIGHEST_PREC) return lhs;
  lhs = parse_infix(lex, arena, lhs, minprec + 1);
  if (!lhs) return NULL;

  // get next token
  Token *next = lexer_peek(lex, 1);
  if (!next) return NULL;

  // while token
  while (next->type == TOKEN_OPERATOR && getprec(getop(next->lexeme)) >= minprec) {
    lexer_consume(lex);
    OperatorType op = getop(next->lexeme);

    // process right hand side operand
    ASTExpr *rhs = parse_factor(lex, arena);
    if (!rhs) return NULL;

    // process higher-precedence tokens
    next = lexer_peek(lex, 1);
    if (!next) return NULL;
    while (next->type == TOKEN_OPERATOR && getprec(getop(next->lexeme)) > minprec) {
      rhs = parse_infix(lex, arena, rhs, minprec + 1);
      next = lexer_peek(lex, 1);
     if (!next) return NULL;
    }

    // new expr
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_BINOP;
    node->val.binop.lhs = lhs;
    node->val.binop.op = op;
    node->val.binop.rhs = rhs;
    lhs = node;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
 }

  return lhs;
}

ASTExpr *parse_factor(Lexer *lex, Arena *arena) {
  Token *next = lexer_peek(lex, 1);
  if (!next)
    return NULL;

  // hierarchy:
  //   a b C d e   ->   (a(b(((C)d)e)))
  //   d -> e -> b -> a
  // unary precedence rules apply:
  //   right -> right-most -> left -> left-most

  // prefix unary operators
  if (next->type == TOKEN_OPERATOR && op_isprefix(getop(next->lexeme))) {
    lexer_consume(lex);

    // make a new node containing the unary op
    ASTExpr *expr = aaloc(arena, ASTExpr);
    if (!expr) return NULL;
    expr->type = AST_EXPR_UNOP;
    expr->val.unop.op = getop(next->lexeme);
    expr->val.unop.isprefix = true;

    // process node value
    ASTExpr *sub = parse_factor(lex, arena);
    if (!sub) return NULL;
    expr->val.unop.val = sub;

    return expr;
  }

  // process primary
  ASTExpr *expr = parse_primary(lex, arena);
  if (!expr) return NULL;
  next = lexer_peek(lex, 1);
  if (!next) return NULL;

  // while there's still postfix ops...
  while (next->type == TOKEN_OPERATOR && op_ispostfix(getop(next->lexeme))) {
    lexer_consume(lex);

    // new node
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_UNOP;
    node->val.unop.op = getop(next->lexeme);
    node->val.unop.isprefix = false;
    node->val.unop.val = expr;
    expr = node;

    // for possible subsequent postfix ops
    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  // check ternary
  if (cmp_token(next, TOKEN_OPERATOR, "?")) {
    lexer_consume(lex); // consume '?'

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_TERNOP;
    node->val.ternop.op = OP_QST;
    node->val.ternop.lch = expr;

    ASTExpr *sub = NULL;

    // true expression
    sub = parse_factor(lex, arena);
    if (!sub) return NULL;
    node->val.ternop.mch = sub;

    // consume colon
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_OPERATOR, ":"))
      return NULL;

    // false expression
    sub = parse_factor(lex, arena);
    if (!sub) return NULL;
    node->val.ternop.rch = sub;

    expr = node;
  }

  return expr;
}

ASTExpr *parse_primary(Lexer *lex, Arena *arena) {
  Token *next = lexer_peek(lex, 1);
  if (!next)
    return NULL;

  // identifier token
  if (next->type == TOKEN_IDENTIFIER)
    return parse_secondary(lex, arena, parse_identifier(lex, arena));

  // string token
  if (next->type == TOKEN_STRING) {
    lexer_consume(lex);
    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_STRING;
    node->val.str.raw = next->lexeme;
    node->val.str.len = next->len;
    return parse_secondary(lex, arena, node);
  }

  // array literals
  if (cmp_token(next, TOKEN_BRACKET, "[")) {
    lexer_consume(lex); // consume '['
    ASTExpr *expr = aaloc(arena, ASTExpr);
    if (!expr) return NULL;
    expr->type = AST_EXPR_ARRAY;
    ASTArray *arr = NULL;

    while (1) {
      next = lexer_peek(lex, 1);
      if (!next) return NULL;

      // maybe end?
      if (cmp_token(next, TOKEN_BRACKET, "]"))
        break;

      // create a new element
      ASTArray *newarr = aaloc(arena, ASTArray);
      if (!newarr) return NULL;
      if (arr) arr->next = newarr;
      arr = newarr;
      arr->expr = NULL;
      arr->next = NULL;
      if (!expr->val.arr) expr->val.arr = arr; // first element

      // process expression (minimum precedence 2 to skip comma)
      ASTExpr *node = parse_infix(lex, arena, parse_factor(lex, arena), 2);
      if (!node) return NULL;
      arr->expr = node;

      // expect ']' or ','
      next = lexer_peek(lex, 1);
      if (!next) return NULL;
      if (
        !cmp_token(next, TOKEN_BRACKET, "]") &&
        !cmp_token(next, TOKEN_OPERATOR, ",")
      ) {
        print_token(next, "syntax error: expected either of ']' and ','\n");
        return NULL;
      }

      // consume if this is a comma
      if (cmp_token(next, TOKEN_OPERATOR, ","))
        lexer_consume(lex);
    }

    // end ']'
    next = lexer_consume(lex);
    if (expect_token(next, TOKEN_BRACKET, "]")) return NULL;
    return parse_secondary(lex, arena, expr);
  }

  // integers
  if (cmp_token(next, TOKEN_INTEGER, NULL)) {
    lexer_consume(lex); // consume the int
    ASTExpr *val = aaloc(arena, ASTExpr);
    if (!val) return NULL;
    val->type = AST_EXPR_INTEGER;
    val->val.intg.text = next->lexeme;
    val->val.intg.len = next->len;
    return parse_secondary(lex, arena, val);
  }

  // type cast
  if (cmp_token(next, TOKEN_OPERATOR, "<")) {
    lexer_consume(lex); // consume '<'
    ASTTypeRef *type = parse_typeref(lex, arena);
    if (!type) return NULL;
    next = lexer_consume(lex); // consume '>'
    if (expect_token(next, TOKEN_OPERATOR, ">")) return NULL;
    ASTExpr *val = parse_factor(lex, arena);
    if (!val) return NULL;
    // setup type cast node
    ASTExpr *node = aaloc(arena, ASTExpr);
    node->type = AST_EXPR_CAST;
    node->val.cast.type = type;
    node->val.cast.val = val;
    return node;
  }

  // expression enclosed with paren
  if (cmp_token(next, TOKEN_BRACKET, "(")) {
    lexer_consume(lex);
    ASTExpr *expr = parse_expr(lex, arena);
    if (!expr) return NULL;
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_BRACKET, ")")) return NULL;
    return parse_secondary(lex, arena, expr);
  }

  expect_token(next, -1, NULL);
  return NULL;
}

ASTExpr *parse_secondary(Lexer *lex, Arena *arena, ASTExpr *lhs) {
  if (!lhs) return NULL;

  Token *next = lexer_peek(lex, 1);
  if (!next)
    return NULL;

  // check whether this is a member-access or subscript op,
  // or a function call
  if (
    !cmp_token(next, TOKEN_OPERATOR, ".") &&
    !cmp_token(next, TOKEN_BRACKET, "[") &&
    !cmp_token(next, TOKEN_BRACKET, "(")
  ) return lhs;

  // check member-access
  while (cmp_token(next, TOKEN_OPERATOR, ".")) {
    lexer_consume(lex); // consume '.'

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_BINOP;
    node->val.binop.op = OP_DOT;
    node->val.binop.lhs = lhs;

    // the member name
    ASTExpr *memb = parse_identifier(lex, arena);
    if (!memb) return NULL;
    node->val.binop.rhs = memb;

    lhs = node;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  // check subscript
  while (cmp_token(next, TOKEN_BRACKET, "[")) {
    lexer_consume(lex); // consume '['

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_BINOP;
    node->val.binop.op = OP_SBC;
    node->val.binop.lhs = lhs;

    // the key
    ASTExpr *sub = parse_expr(lex, arena);
    if (!sub) return NULL;
    node->val.binop.rhs = sub;

    // consume ']'
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_BRACKET, "]"))
      return NULL;

    lhs = node;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  // check function calls
  while (cmp_token(next, TOKEN_BRACKET, "(")) {
    lexer_consume(lex); // consume '('

    ASTExpr *node = aaloc(arena, ASTExpr);
    if (!node) return NULL;
    node->type = AST_EXPR_CALL;
    node->val.fcall.fname = lhs;
    ASTFuncArg *curr = NULL;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;

    // the args
    while (!cmp_token(next, TOKEN_BRACKET, ")")) {
      // initialize arg
      ASTFuncArg *arg = aaloc(arena, ASTFuncArg);
      if (!arg) return NULL;
      arg->target = NULL;
      arg->tlen = 0;

      // kwarg?
      if (
        cmp_token(next, TOKEN_IDENTIFIER, NULL) &&
        cmp_token(lexer_peek(lex, 2), TOKEN_OPERATOR, "=") &&
        lexer_peek(lex, 2)->pos == next->pos + next->len
        // the id should be close to the equal sign to consider it as a kwarg
      ) {
        lexer_consume(lex); // consume id
        lexer_consume(lex); // consume '='
        arg->target = next->lexeme;
        arg->tlen = next->len;
      }

      // process arg expression
      ASTExpr *val = parse_infix(lex, arena, parse_factor(lex, arena), 2);
      if (!val) return NULL;
      arg->val = val;

      if (curr) curr->next = arg;
      else node->val.fcall.args = arg;
      curr = arg;

      // peek next token
      next = lexer_peek(lex, 1);
      if (!next) return NULL;

      // next arg
      if (cmp_token(next, TOKEN_OPERATOR, ",")) {
        lexer_consume(lex);
        next = lexer_peek(lex, 1);
        if (!next) return NULL;

        // ')' after ',' ??
        if (cmp_token(next, TOKEN_BRACKET, ")")) {
          expect_token(next, -1, NULL);
          return NULL;
        }

        continue;
      }

      // found end
      if (cmp_token(next, TOKEN_BRACKET, ")"))
        break;
    }

    // consume ')'
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_BRACKET, ")"))
      return NULL;

    lhs = node;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  return parse_secondary(lex, arena, lhs);
}

ASTStm *parse_statement(Lexer *lex, Arena *arena) {
  Token *next = lexer_peek(lex, 1);
  if (!next) return NULL;
  ASTStm *stm = aaloc(arena, ASTStm);
  if (!stm) return NULL;
  stm->next = NULL; // used on blocks

  if (next->type == TOKEN_KEYWORD) {
    lexer_consume(lex); // consume the keyword
    KeywordType kwd = getkwd(next->lexeme);

    switch (kwd) {
      case KWD_LET: {
        stm->type = AST_STM_LET;

        // process type
        ASTTypeRef *type = parse_typeref(lex, arena);
        if (!type) return NULL;
        stm->val.let.type = type;

        // get identifier
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_IDENTIFIER, NULL))
          return NULL;
        stm->val.let.name = next->lexeme;
        stm->val.let.nlen = next->len;

        // check whether there is initial value
        next = lexer_consume(lex);
        if (!next) return NULL;
        stm->val.let.initval = NULL;

        if (cmp_token(next, TOKEN_OPERATOR, "=")) {
          ASTExpr *initval = parse_expr(lex, arena);
          if (!initval) return NULL;
          stm->val.let.initval = initval;
          next = lexer_consume(lex);
          if (!next) return NULL;
        }

        // expect semi-colon
        if (expect_token(next, TOKEN_DELIMETER, ";"))
          return NULL;
        return stm;
      }

      case KWD_IF: {
        stm->type = AST_STM_IFELSE;

        // expect condition opening '('
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, "("))
          return NULL;

        ASTExpr *cond = parse_expr(lex, arena);
        if (!cond) return NULL;
        stm->val.ifels.cond = cond;

        // expect condition closing ')'
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, ")"))
          return NULL;

        // now the code to execute
        ASTStm *code = parse_statement(lex, arena);
        if (!code) return NULL;
        stm->val.ifels.code = code;

        // the else statement
        stm->val.ifels.elsec = NULL;
        next = lexer_peek(lex, 1);
        if (!next) return NULL;
        if (cmp_token(next, TOKEN_KEYWORD, "else")) {
          lexer_consume(lex); // consume 'else'
          ASTStm *elsec = parse_statement(lex, arena);
          if (!elsec) return NULL;
          stm->val.ifels.elsec = elsec;
        }

        return stm;
      }

      case KWD_ELSE: {
        print_token(next, "syntax error: the 'else' statement must be preceded by an 'if' statement\n");
        return NULL;
      }

      case KWD_WHILE: {
        stm->type = AST_STM_WHILE;

        // expect condition opening '('
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, "("))
          return NULL;

        ASTExpr *cond = parse_expr(lex, arena);
        if (!cond) return NULL;
        stm->val.whil.cond = cond;

        // expect condition closing ')'
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_BRACKET, ")"))
          return NULL;

        // code to execute
        ASTStm *code = parse_statement(lex, arena);
        if (!code) return NULL;
        stm->val.whil.code = code;

        return stm;
      }

      case KWD_RETURN: {
        stm->type = AST_STM_RETURN;
        stm->val.retval = NULL;

        // check whether there's return value
        next = lexer_peek(lex, 1);
        if (!next) return NULL;
        if (!cmp_token(next, TOKEN_DELIMETER, ";")) {
          ASTExpr *retval = parse_expr(lex, arena);
          if (!retval) return NULL;
          stm->val.retval = retval;
        }

        // expect delimeter
        next = lexer_consume(lex);
        if (!next) return NULL;
        if (expect_token(next, TOKEN_DELIMETER, ";"))
          return NULL;

        return stm;
      }

      default:
        // primitive type keyword
        if (iskwdprim(kwd)) {
          expect_token(next, -1, NULL);
          return NULL;
        }

        // it is impossible to get unknown keyword, but just in-case :)
        print_token(next, "parse error: unknown keyword\n");
        return NULL;
    }
  }

  // blocks
  if (cmp_token(next, TOKEN_BRACKET, "{")) {
    stm->type = AST_STM_BLOCK;
    ASTBlock *block = parse_block(lex, arena);
    if (!block) return NULL;
    stm->val.blck = block;
    return stm;
  }

  // expressions
  stm->type = AST_STM_EXPR;
  ASTExpr *expr = parse_expr(lex, arena);
  if (!expr) return NULL;
  stm->val.expr = expr;

  // semi-colon
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_DELIMETER, ";"))
    return NULL;

  return stm;
}

ASTBlock *parse_block(Lexer *lex, Arena *arena) {
  Token *next = NULL;

  // expect block opening '{'
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_BRACKET, "{"))
    return NULL;

  // initialize node
  ASTBlock *node = aaloc(arena, ASTBlock);
  if (!node) return NULL;
  ASTStm *head = NULL;
  ASTStm *tail = NULL;

  // process until closing bracket '}'
  next = lexer_peek(lex, 1);
  if (!next) return NULL;
  while (!cmp_token(next, TOKEN_BRACKET, "}") && !lex->eof) {
    ASTStm *stm = parse_statement(lex, arena);
    if (!stm) return NULL;

    // update head and tail pointers
    if (!head) head = stm;
    if (tail) tail->next = stm;
    tail = stm;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  // set pointers
  node->head = head;
  node->tail = tail;

  // expect block closing '}'
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_BRACKET, "}"))
    return NULL;

  return node;
}

ASTFuncDef *parse_funcdef(Lexer *lex, Arena *arena) {
  ASTFuncDef *fn = aaloc(arena, ASTFuncDef);
  if (!fn) return NULL;
  fn->args = NULL;
  fn->code = NULL;
  Token *next = lexer_consume(lex);
  if (!next) return NULL;

  // expect declaration keyword
  if (expect_token(next, TOKEN_KEYWORD, "function"))
    return NULL;
  next = lexer_peek(lex, 1);
  if (!next) return NULL;

  // TODO: handle generic parameters
  // function <T = auto> T getFirst(T[] arr);

  // get return type
  ASTTypeRef *rettype = parse_typeref(lex, arena);
  if (!rettype) return NULL;
  fn->rettype = rettype;

  // get function id
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_IDENTIFIER, NULL))
    return NULL;
  fn->name = next->lexeme;
  fn->nlen = next->len;

  // expect arg opening
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_BRACKET, "("))
    return NULL;

  // process args
  ASTFuncArgDef *curr = NULL;
  bool defargs = false; // whether expect args to have defaukt value
  next = lexer_peek(lex, 1);
  if (!next) return NULL;
  while (!cmp_token(next, TOKEN_BRACKET, ")")) {
    ASTFuncArgDef *arg = aaloc(arena, ASTFuncArgDef);
    if (!arg) return NULL;
    arg->next = NULL;
    arg->defval = NULL;
    arg->restarr = false;

    if (curr) curr->next = arg;
    else fn->args = arg;
    curr = arg;

    // get arg type
    ASTTypeRef *argtype = parse_typeref(lex, arena);
    if (!argtype) return NULL;
    arg->type = argtype;

    // get arg id
    next = lexer_consume(lex);
    if (expect_token(next, TOKEN_IDENTIFIER, NULL))
      return NULL;
    arg->name = next->lexeme;
    arg->nlen = next->len;

    // next token
    next = lexer_peek(lex, 1);
    if (!next) return NULL;

    // rest args indicator
    if (cmp_token(next, TOKEN_OPERATOR, "...")) {
      lexer_consume(lex); // ...
      arg->restarr = true;
      break;
    }

    // default value
    if (cmp_token(next, TOKEN_OPERATOR, "=")) {
      lexer_consume(lex);
      // parse expression with precedence 2 (exclude comma operator)
      ASTExpr *defval = parse_infix(lex, arena, parse_factor(lex, arena), 2);
      if (!defval) return NULL;
      arg->defval = defval;
      defargs = true;
      next = lexer_peek(lex, 1);
      if (!next) return NULL;
    }

    // required arg after optional ones
    else if (defargs && (
      cmp_token(next, TOKEN_OPERATOR, ",") ||
      cmp_token(next, TOKEN_BRACKET, ")")
    )) {
      print_token(lexer_peek(lex, 0),
        "syntax error: unexpected required argument after optional parameters\n");
      return NULL;
    }

    // next arg
    if (cmp_token(next, TOKEN_OPERATOR, ",")) {
      lexer_consume(lex);
      continue;
    }

    // end arg defs
    if (cmp_token(next, TOKEN_BRACKET, ")"))
      break;
  }

  // expect closing ')'
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_BRACKET, ")"))
    return NULL;

  // TODO: thrown error types
  // function void someErr() noexcept;

  next = lexer_peek(lex, 1);
  if (!next) return NULL;

  // function definition
  if (cmp_token(next, TOKEN_BRACKET, "{")) {
    ASTBlock *code = parse_block(lex, arena);
    if (!code) return NULL;
    fn->code = code;
    return fn;
  }

  // just a declaration
  lexer_consume(lex);
  if (expect_token(next, TOKEN_DELIMETER, ";"))
    return NULL;
  return fn;
}

ASTEnum *parse_enum(Lexer *lex, Arena *arena) {
  ASTEnum *enode = aaloc(arena, ASTEnum);
  if (!enode) return NULL;
  enode->head = NULL;
  enode->tail = NULL;
  enode->type = NULL;
  Token *next = lexer_consume(lex);
  if (!next) return NULL;

  // expect 'enum'
  if (expect_token(next, TOKEN_KEYWORD, "enum"))
    return NULL;

  // get enum id
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_IDENTIFIER, NULL))
    return NULL;
  enode->name = next->lexeme;
  enode->nlen = next->len;

  // type or definition
  next = lexer_peek(lex, 1);
  if (!next) return NULL;

  // type found
  if (!cmp_token(next, TOKEN_BRACKET, "{")) {
    ASTTypeRef *type = parse_typeref(lex, arena);
    if (!type) return NULL;
    enode->type = type;
    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  // definition of the enum
  if (expect_token(next, TOKEN_BRACKET, "{"))
    return NULL;
  lexer_consume(lex); // consume '{'
  next = lexer_peek(lex, 1);
  if (!next) return NULL;

  ASTEnumEntry *curr = NULL;
  while (!cmp_token(next, TOKEN_BRACKET, "}")) {
    ASTEnumEntry *ent = aaloc(arena, ASTEnumEntry);
    if (!ent) return NULL;
    ent->next = NULL;
    ent->cnst = NULL;

    if (curr) curr->next = ent;
    else enode->head = ent;
    curr = ent;

    // get enum name
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_IDENTIFIER, NULL))
      return NULL;
    ent->name = next->lexeme;
    ent->nlen = next->len;

    // process constant value
    next = lexer_peek(lex, 1);
    if (!next) return NULL;
    if (cmp_token(next, TOKEN_OPERATOR, "=")) {
      lexer_consume(lex); // consume '='
      // parse expression, min prec 2 to exclude comma
      ASTExpr *cnst = parse_infix(lex, arena, parse_factor(lex, arena), 2);
      if (!cnst) return NULL;
      ent->cnst = cnst;
      next = lexer_peek(lex, 1);
      if (!next) return NULL;
    }

    // if there's no comma, it is the last element
    if (!cmp_token(next, TOKEN_OPERATOR, ","))
      break;

    lexer_consume(lex); // consume ','
    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }
  enode->tail = curr;

  // no enum entry was processed
  if (!curr) {
    print_token(next, "syntax error: empty enum definition not allowed\n");
    return NULL;
  }

  // expect closing '}'
  next = lexer_consume(lex);
  if (!next) return NULL;
  if (expect_token(next, TOKEN_BRACKET, "}"))
    return NULL;

  return enode;
}

ASTTypeRef *parse_typeref(Lexer *lex, Arena *arena) {
  ASTTypeRef *node = aaloc(arena, ASTTypeRef);
  if (!node) return NULL;
  Token *next = lexer_consume(lex);
  if (!next) return NULL;

  // a primitive type
  if (next->type == TOKEN_KEYWORD && iskwdprim(getkwd(next->lexeme))) {
    node->type = AST_TYPE_PRIMITIVE;
    node->val.type = kwdtoprim(getkwd(next->lexeme));
  }

  // unknown type token
  else {
    expect_token(next, -1, NULL);
    return NULL;
  }

  // process array types
  next = lexer_peek(lex, 1);
  if (!next) return NULL;
  while (cmp_token(next, TOKEN_BRACKET, "[")) {
    lexer_consume(lex); // consume '['

    // expect ']'
    next = lexer_consume(lex);
    if (!next) return NULL;
    if (expect_token(next, TOKEN_BRACKET, "]"))
      return NULL;

    ASTTypeRef *ref = aaloc(arena, ASTTypeRef);
    if (!ref) return NULL;
    ref->type = AST_TYPE_ARRAY;
    ref->val.aelem = node;
    node = ref;

    next = lexer_peek(lex, 1);
    if (!next) return NULL;
  }

  return node;
}

#ifdef _DEBUG
void print_expr(ASTExpr *expr) {
  if (!expr) {
    printf("[null]");
    return;
  }

  switch (expr->type) {
    case AST_EXPR_UNOP:
      if (expr->val.unop.isprefix) printf("%s", OperatorNames[expr->val.unop.op]);
      fputc('(', stdout);
      print_expr(expr->val.unop.val);
      fputc(')', stdout);
      if (!expr->val.unop.isprefix) printf("%s", OperatorNames[expr->val.unop.op]);
      break;
    case AST_EXPR_BINOP:
      fputc('(', stdout);
      print_expr(expr->val.binop.lhs);
      if (expr->val.binop.op == OP_SBC) {
        fputc('[', stdout);
        print_expr(expr->val.binop.rhs);
        fputc(']', stdout);
      } else {
        printf(" %s ", OperatorNames[expr->val.binop.op]);
        print_expr(expr->val.binop.rhs);
      }
      fputc(')', stdout);
      break;
    case AST_EXPR_TERNOP:
      fputc('(', stdout);
      if (expr->val.ternop.op != OP_QST) {
        printf("null)");
        return;
      }
      print_expr(expr->val.ternop.lch);
      printf(" ? ");
      print_expr(expr->val.ternop.mch);
      printf(" : ");
      print_expr(expr->val.ternop.rch);
      fputc(')', stdout);
      break;
    case AST_EXPR_IDENTIFIER:
      pview(expr->val.ident.name, expr->val.ident.len);
      break;
  }
}
#endif // _DEBUG

