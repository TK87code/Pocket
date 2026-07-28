#ifndef PKT_MEMORY_INTERNAL_H
#define PKT_MEMORY_INTERNAL_H

struct pkt_arena;

int __pkt_memory_init(struct pkt_arena *arena, void *backing_buffer, size_t arena_size);
void *__pkt_memory_reserve(struct pkt_arena *arena, size_t bytes); 
int __pkt_memory_release(struct pkt_arena *arena); 
	
#endif
