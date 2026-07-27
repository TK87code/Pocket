#include <stddef.h> // size_t
#include <stdlib.h> // malloc, free
#include "pocket.h"

int p_arena_init(struct p_arena *arena, void *backing_buffer, size_t arena_size)
{
	arena->base = backing_buffer;
	arena->size = arena_size;
	arena->offset = 0;

	return 0;
}

void *p_arena_alloc(struct p_arena *arena, size_t bytes) 
{
	size_t remaining = arena->size - arena->offset;
	if (remaining < bytes)
		return NULL;

	void *ptr = arena->base + arena->offset;
	arena->offset += bytes;

	return ptr;
}

int p_arena_clear(struct p_arena *arena) 
{
	arena->offset = 0;

	return 0;
}
