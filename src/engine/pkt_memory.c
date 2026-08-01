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

void *__pkt_memory_alloc(struct pkt_arena *arena, size_t bytes) 
{
	size_t remaining = arena->size - arena->offset;
	if (remaining < bytes)
		return NULL;

	void *ptr = arena->base + arena->offset;
	arena->offset += bytes;

	return ptr;
}

int __pkt_memory_reset(struct pkt_arena *arena) 
{
	arena->offset = 0;

	return 0;
}

int __pkt_memory_pool_init(struct pkt_pool_manager *manager, void *backing_buffer, size_t block_size, size_t max_elems)
{
	manager->base = (unsigned char *)backing_buffer;
	manager->block_size = block_size;
	manager->max_elems = max_elems;

	for (size_t i = 0; i < max_elems - 1; i++) {
		unsigned char *current_addr = manager->base + (i * block_size);
		unsigned char *next_addr = manager->base + ((i + 1) * block_size);
		struct pkt_pool_node *current_node = (struct pkt_pool_node *)current_addr;
	
		current_node->next = (struct pkt_pool_node *)next_addr;
	}

	unsigned char *last_addr = manager->base + ((max_elems - 1) * block_size);
	struct pkt_pool_node *last_node = (struct pkt_pool_node *)last_addr;
	last_node->next = NULL;	

	manager->head = (struct pkt_pool_node *)manager->base;
	
	return 0;
}


void *__pkt_memory_pool_alloc(struct pkt_pool_manager *manager)
{
	if (manager->head == NULL)
		return NULL;
	struct pkt_pool_node *node = manager->head;
	manager->head = node->next;	

	return node;
}


int __pkt_memory_pool_free(struct pkt_pool_manager *manager, void *ptr)
{
	if (ptr == NULL)
		return -1;

	struct pkt_pool_node *n = (struct pkt_pool_node *)ptr;
	n->next = manager->head;
	manager->head = n;

	return 0;
}
