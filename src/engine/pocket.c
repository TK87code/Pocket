#include <stdio.h>
#include <time.h>    // clock_gettime
#include <unistd.h>  // usleep
#include <stdarg.h>
#include "pocket.h"
#include "pkt_terminal_internal.h"
#include "pkt_scene_internal.h"
#include "pkt_event_internal.h"
#include "pkt_memory_internal.h"

#define PKT_DEFAULT_TARGET_FPS 60
#define PKT_BACKING_BUFFER_SIZE (1024 * 1024)

static int __pkt_load_config(struct pkt_config *config);

// Reserve memory for frame_arena in BSS segment
static unsigned char backing_buffer[PKT_BACKING_BUFFER_SIZE]; 
static struct pkt_arena frame_arena;

static int is_running = 1; 
static int target_fps;

struct pkt_config pkt_get_default_config(void)
{
	struct pkt_config c = {0};
	c.target_fps = PKT_DEFAULT_TARGET_FPS;
	c.screen_col = PKT_DEFAULT_SCOL;
	c.screen_row = PKT_DEFAULT_SROW;
	c.default_fcolor = PKT_COLOR_WHITE;
	c.default_bcolor = PKT_COLOR_BLACK;
	c.user_data = NULL;
	c.on_init = NULL;

	return c;
}

int pkt_init(struct pkt_config* config)
{	
	if (__pkt_terminal_init(config) < 0)
		return -1;
	__pkt_memory_init(&frame_arena, backing_buffer, (size_t)PKT_BACKING_BUFFER_SIZE); 
	
	if (__pkt_load_config(config) < 0)
		return -2;
	 
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

		int k = __pkt_terminal_read_input();
		if (k != -1) {
			struct pkt_event e;
			e.type = PKT_EVENT_KEY_PRESSED;
			e.data.key.key_code = k;
			__pkt_event_push(&e);
		}

		__pkt_scene_update(dt);
		__pkt_scene_draw();

		__pkt_terminal_update();
		__pkt_memory_reset(&frame_arena);

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

void *pkt_scratch_alloc(size_t bytes)
{
	return __pkt_memory_alloc(&frame_arena, bytes);	
}

int pkt_poll_event(struct pkt_event *event)
{
	return __pkt_event_poll(event);
}

int pkt_putc(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, char c)
{
	return __pkt_terminal_putc(x, y, fcolor, bcolor, c);
}

int pkt_puts(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, const char *str)
{
	return __pkt_terminal_puts(x, y, fcolor, bcolor, str);
}

int pkt_printf(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, const char *fmt, ...)
{
	char buf[512] = {0};
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	return __pkt_terminal_puts(x, y, fcolor, bcolor, buf);
}

int pkt_register_scene(int scene_id, const struct pkt_scene *scene)
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

int pkt_init_arena(struct pkt_arena *arena, void *backing_buffer, size_t arena_size)
{
	return __pkt_memory_init(arena, backing_buffer, arena_size);
}

void *pkt_arena_alloc(struct pkt_arena *arena, size_t bytes)
{
	return __pkt_memory_alloc(arena, bytes);
}

int pkt_reset_arena(struct pkt_arena *arena)
{
	return __pkt_memory_reset(arena);
}

// === Static functions ===

static int __pkt_load_config(struct pkt_config *config)
{
	if (config == NULL || config->on_init == NULL)
		return -1;
	else
		config->on_init(config->user_data);

	if (config->target_fps <= 0 || config->target_fps > 360)
		target_fps = PKT_DEFAULT_TARGET_FPS;
	else
		target_fps = config->target_fps;

	return 0;
}
