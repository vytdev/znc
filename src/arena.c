#include "arena.h"
#include "types.h"
#include <stdlib.h>

Arena *arena_init(uvar bsize) {
  if (bsize == 0) return NULL;
  bsize = bsize > ARENA_MINSIZE ? bsize : ARENA_MINSIZE;

  // allocate arena
  Arena *arena = (Arena*)malloc(sizeof(Arena) + bsize);
  if (!arena) return NULL;

  // init fields
  arena->next = NULL;
  arena->used = 0;
  arena->alloc = bsize;
  return arena;
}

void arena_free(Arena *arena) {
  if (!arena) return;
  arena_free(arena->next);
  arena->next = NULL;
  arena->used = 0;
  arena->alloc = 0;
}

void *arena_reqm(Arena *arena, uvar size) {
  if (!arena || size == 0) return NULL;
  Arena *block, *tail;

  // find a suitable block for this allocation
  block = arena;
  while (block && block->alloc < block->used + size) {
    tail = block;
    block = block->next;
  }

  // attempt make a new block
  if (!block) {
    uvar newsz = tail->alloc;
    while (newsz < size) newsz *= 2;
    block = arena_init(newsz);
    if (!block) return NULL;
    tail->next = block;
  }

  // allocate memory
  char *curr = &block->block[block->used];
  block->used += size;
  return curr;
}

