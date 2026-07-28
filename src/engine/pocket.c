#include <time.h>    // clock_gettime
#include <unistd.h>  // usleep
#include "pocket.h"
#include "p_terminal_internal.h"
#include "p_scene_internal.h"
#include "p_event_internal.h"

#define DEFAULT_TARGET_FPS 60
#define BACKING_BUFFER_SIZE 1024 * 1024

static int __pocket_load_config(struct p_game_config *config);

// Reserve memory for frame_arena in BSS segment
static unsigned char backing_buffer[BACKING_BUFFER_SIZE]; 
static struct p_arena frame_arena;

static int is_running = 1; 
static int target_fps;

int pocket_init(struct p_game_config* config)
{	
	if (__pocket_load_config(config) < 0)
		return -1;

	__p_terminal_init();
	p_arena_init(&frame_arena, backing_buffer, (size_t)BACKING_BUFFER_SIZE);  
	return 0;
}

int pocket_ignite(void)
{
	useconds_t target_usec = 1000000 / target_fps; // 1 sec = 1,000,000 micro sec
	struct timespec s_time, e_time;
	useconds_t elapsed_usec = 0;

	while(is_running) {
		float dt = elapsed_usec / 1000000.0f;
		(void)clock_gettime(CLOCK_MONOTONIC, &s_time);

		int k = __p_terminal_getch();
		if (k != -1) {
			struct p_event e;
			e.type = P_EVENT_KEY_PRESSED;
			e.data.key.key_code = k;
			__p_event_push(&e);
		}

		__p_scene_update(dt);
		__p_scene_draw();

		__p_terminal_update();
		p_arena_release(&frame_arena);

		(void)clock_gettime(CLOCK_MONOTONIC, &e_time);
		elapsed_usec = (e_time.tv_sec - s_time.tv_sec) * 1000000 // sec to microsec
			+ (e_time.tv_nsec - s_time.tv_nsec) / 1000; // nanosec to microsec

		if (target_usec > elapsed_usec)
			(void)usleep(target_usec - elapsed_usec);
	}

	return 0;
}

int pocket_quit(void)
{
	is_running = 0;

	return 0;
}

int pocket_cleanup(void)
{
	__p_terminal_restore();

	return 0;
}

void *pocket_reserve_frame_arena(size_t bytes)
{
	return p_arena_reserve(&frame_arena, bytes);	
}

int pocket_poll_event(struct p_event *event)
{
	return __p_event_poll(event);
}

static int __pocket_load_config(struct p_game_config *config)
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
