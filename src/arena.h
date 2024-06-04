#ifndef _ZNC_ARENA_H
#define _ZNC_ARENA_H
#include "types.h"

#define ARENA_MINSIZE 65536 /* 64kiB */

typedef struct Arena {
  struct Arena *next;
  uvar used;
  uvar alloc;
  char block[];
} Arena;

/* initialize an arena block given block size */
Arena *arena_init(uvar bsize);

/* free arena */
void arena_free(Arena *arena);

/* get memory from arena */
void *arena_reqm(Arena *arena, uvar size);

// just an alias
#define aaloc(arena, type) (type*)arena_reqm(arena, sizeof(type))

#endif // _ZNC_ARENA_H

