#include <time.h>    // clock_gettime
#include <unistd.h>  // usleep
#include "pocket.h"
#include "pkt_terminal_internal.h"
#include "pkt_scene_internal.h"
#include "pkt_event_internal.h"
#include "pkt_memory_internal.h"

#define DEFAULT_TARGET_FPS 60
#define BACKING_BUFFER_SIZE 1024 * 1024

static int __pkt_load_config(struct pkt_config *config);

// Reserve memory for frame_arena in BSS segment
static unsigned char backing_buffer[BACKING_BUFFER_SIZE]; 
static struct pkt_arena frame_arena;

static int is_running = 1; 
static int target_fps;

int pkt_init(struct pkt_config* config)
{	
	if (__pkt_load_config(config) < 0)
		return -1;

	__pkt_terminal_init();
	pkt_init_memory_arena(&frame_arena, backing_buffer, (size_t)BACKING_BUFFER_SIZE);  
	return 0;
}

int pkt_ignite(void)
{
	useconds_t target_usec = 1000000 / target_fps; // 1 sec = 1,000,000 micro sec
	struct timespec s_time, e_time;
	useconds_t elapsed_usec = 0;

	while(is_running) {
		float dt = elapsed_usec / 1000000.0f;
		(void)clock_gettime(CLOCK_MONOTONIC, &s_time);

		int k = __pkt_terminal_getch();
		if (k != -1) {
			struct pkt_event e;
			e.type = PKT_EVENT_KEY_PRESSED;
			e.data.key.key_code = k;
			__pkt_event_push(&e);
		}

		__pkt_scene_update(dt);
		__pkt_scene_draw();

		__pkt_terminal_update();
		pkt_release_memory(&frame_arena);

		(void)clock_gettime(CLOCK_MONOTONIC, &e_time);
		elapsed_usec = (e_time.tv_sec - s_time.tv_sec) * 1000000 // sec to microsec
			+ (e_time.tv_nsec - s_time.tv_nsec) / 1000; // nanosec to microsec

		if (target_usec > elapsed_usec)
			(void)usleep(target_usec - elapsed_usec);
	}

	return 0;
}

int pkt_quit(void)
{
	is_running = 0;

	return 0;
}

// === Wrapper functions ===

int pkt_cleanup(void)
{
	return __pkt_terminal_restore();
}

void *pkt_reserve_scratch_memory(size_t bytes)
{
	return __pkt_memory_reserve(&frame_arena, bytes);	
}

int pkt_poll_event(struct pkt_event *event)
{
	return __pkt_event_poll(event);
}

int pkt_putch(int x, int y, int color, char c)
{
	return __pkt_terminal_putch(x, y, color, c);
}

int pkt_putstr(int x, int y, int color, const char *str)
{
	return __pkt_terminal_putstr(x, y, color, str);
}

int pkt_register_scene(int scene_id, struct pkt_scene *scene)
{
	return __pkt_scene_register(scene_id, scene);
}

int pkt_swap_scene(int next_scene_id)
{
	return __pkt_scene_swap(next_scene_id);
}

int pkt_get_scene(void)
{
	return __pkt_scene_get();
}

int pkt_init_memory_arena(struct pkt_arena *arena, void *backing_buffer, size_t arena_size)
{
	return __pkt_memory_init(arena, backing_buffer, arena_size);
}

void *pkt_reserve_memory(struct pkt_arena *arena, size_t bytes)
{
	return __pkt_memory_reserve(arena, bytes);
}

int pkt_release_memory(struct pkt_arena *arena)
{
	return __pkt_memory_release(arena);
}

// === Static functions ===

static int __pkt_load_config(struct pkt_config *config)
{
	if (config == NULL || config->on_init == NULL)
		return -1;
	else
		config->on_init(config->user_data);

	if (config->target_fps <= 0)
		target_fps = DEFAULT_TARGET_FPS;
	else
		target_fps = config->target_fps;

	return 0;
}
