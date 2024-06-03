#include "test.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifndef _TESTSUITE
#error unknown test suite name
#endif

// test entry
typedef struct {
  const char   *name;
  TEST__fn      func;
  bool          ran;
  bool          passed;
} testEntry;

// the register
static testEntry *entries = NULL;
static int   entriesCount = 0;
static int   entriesAlloc = 0;

// error message
static inline void msg(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  fputc('\n', stderr);
  va_end(args);
}

// test() function declaration, you should implement this
int test(const char *suite);


int main(void) {
  // setup the registry
  entries = (testEntry*)malloc(sizeof(testEntry));
  if (!entries) {
    msg("fatal: failed to init framework");
    return 1;
  }
  entriesAlloc = 1;
  entries[0].name = NULL;

  // some msg
  fputc('\n', stderr);
  msg("==================================");
  msg("SUITE NAME: %s", _TESTSUITE);

  // call the test function
  int retc = test(_TESTSUITE);

  // report result
  int ran    = 0;
  int passed = 0;
  int failed = 0;
  for (int i = 0; i < entriesCount; i++) {
    testEntry *ent = &entries[i];
    if (!ent->name)
      continue;
    if (!ent->ran) {
      msg("[case] skipped %s", ent->name);
      continue;
    }
    // record stat
    ran++;
    if (ent->passed) passed++;
    else             failed++;
  }

  msg("TEST STAT:");
  msg("  ran:     %d", ran);
  msg("  skipped: %d", entriesCount - ran);
  msg("  passed:  %d", passed);
  msg("  failed:  %d", failed);
  msg("==================================");
  fputc('\n', stderr);

  free(entries);
  return failed > 0 && retc == 0 ? 1 : retc;
}

int TEST__register(const char *name, TEST__fn fn) {
  if (entriesCount >= entriesAlloc) {
    testEntry *tmp = (testEntry*)realloc(entries, entriesAlloc * 2);
    if (!tmp) {
      msg("error: failed to register test: %s", name);
      return 1;
    }
    for (int i = 0; i < entriesAlloc; i++)
      tmp[entriesAlloc + i].name = NULL;
    entriesAlloc *= 2;
    entries = tmp;
  }

  // find a free slot
  for (int i = 0; i < entriesAlloc; i++) {
    if (!entries[i].name) {
      entries[i].name   = name;
      entries[i].func   = fn;
      entries[i].ran    = false;
      entries[i].passed = false;
      entriesCount++;
      return 0;
    }
  }

  // edge case!
  return 1;
}

int TEST__deregister(const char *name) {
  // find the test
  for (int i = 0; i < entriesCount; i++) {
    if (entries[i].name && strcmp(name, entries[i].name) == 0) {
      entries[i].name = NULL;
      entriesCount--;
      return 0;
    }
  }

  msg("warn: attempt to deregister an unregistered test: %s", name);
  return 1;
}

int TEST__run(const char *name) {
  // find and run the test
  for (int i = 0; i < entriesCount; i++) {
    if (entries[i].name && strcmp(name, entries[i].name) == 0) {
      msg("[case] running %s ...", name);
      int ret = entries[i].func();
      entries[i].ran    = true;
      entries[i].passed = ret == 0;
      if (ret != 0) msg("[case] test %s failed", name);
      else          msg("[case] test %s passed", name);
      return ret;
    }
  }

  msg("error: cannot run undefined test: %s", name);
  return 1;

}

int TEST__assert(int cond, const char *fail_msg, ...) {
  if (!cond) {
    va_list args;
    va_start(args, fail_msg);
    fprintf(stderr, "[assert] failed: ");
    vfprintf(stderr, fail_msg, args);
    fputc('\n', stderr);
    va_end(args);
  }
  return cond;
}

