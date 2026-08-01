#ifndef PKT_MEMORY_INTERNAL_H
#define PKT_MEMORY_INTERNAL_H

struct pkt_arena;
struct pkt_pool_manager;

int __pkt_memory_init(struct pkt_arena *arena, void *backing_buffer, size_t arena_size);
void *__pkt_memory_alloc(struct pkt_arena *arena, size_t bytes); 
int __pkt_memory_reset(struct pkt_arena *arena); 
int __pkt_memory_pool_init(struct pkt_pool_manager *manager, void *backing_buffer, size_t block_size, size_t max_elems);
void *__pkt_memory_pool_alloc(struct pkt_pool_manager *manager);
int __pkt_memory_pool_free(struct pkt_pool_manager *manager, void *ptr);
	
#endif
