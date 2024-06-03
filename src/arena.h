#ifndef _ZNC_ARENA_H
#define _ZNC_ARENA_H
#include "types.h"

typedef struct {
  char *block;
  uvar alloc;
  uvar used;
} Arena;

/* initialize an arena, returns true if succeded */
int arena_init(Arena *arena);

/* free arena */
void arena_free(Arena *arena);

/* get memory from arena */
void *arena_reqm(Arena *arena, uvar size);

// just an alias
#define aaloc(arena, type) (type*)arena_reqm(arena, sizeof(type))

#endif // _ZNC_ARENA_H

