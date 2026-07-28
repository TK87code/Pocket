#include <stddef.h> // size_t
#include <stdlib.h> // malloc, free
#include "pkt_memory_internal.h"
#include "pocket.h"

int __pkt_memory_init(struct pkt_arena *arena, void *backing_buffer, size_t arena_size)
{
	arena->base = backing_buffer;
	arena->size = arena_size;
	arena->offset = 0;

	return 0;
}

void *__pkt_memory_reserve(struct pkt_arena *arena, size_t bytes) 
{
	size_t remaining = arena->size - arena->offset;
	if (remaining < bytes)
		return NULL;

	void *ptr = arena->base + arena->offset;
	arena->offset += bytes;

	return ptr;
}

int __pkt_memory_release(struct pkt_arena *arena) 
{
	arena->offset = 0;

	return 0;
}
