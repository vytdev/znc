#include "arena.h"
#include "types.h"
#include <stdlib.h>

int arena_init(Arena *arena) {
  if (!arena)
    return 0;
  arena->block = (char*)malloc(1024); // 1kiB
  if (!arena->block)
    return 0;
  arena->alloc = 1024;
  arena->used  = 0;
  return 1;
}

void arena_free(Arena *arena) {
  if (!arena)
    return;
  if (arena->block)
    free(arena->block);
  arena->block = NULL;
  arena->alloc = 0;
  arena->used  = 0;
}

void *arena_reqm(Arena *arena, uvar size) {
  if (!arena || !arena->block || arena->alloc == 0)
    return NULL;

  // arena is full
  if (arena->alloc <= arena->used + size) {
    uvar newsz = arena->alloc;
    while (newsz <= arena->used + size)
      newsz *= 2;
    char *tmp = (char*)realloc(arena->block, newsz);
    if (!tmp)
      return NULL;
    arena->block = tmp;
    arena->alloc = newsz;
  }

  // get memory
  char *curr = &arena->block[arena->used];
  arena->used += size;
  return curr;
}

