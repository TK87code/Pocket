#include <time.h>    // clock_gettime
#include <unistd.h>  // usleep
#include "pocket.h"
#include "p_input_internal.h"
#include "p_screen_internal.h"
#include "p_scene_internal.h"

#define DEFAULT_TARGET_FPS 60

static int __p_config_load(struct p_game_config *config);

static int is_running = 1; 
static int target_fps;

void p_engine_ignite(void)
{
	useconds_t target_usec = 1000000 / target_fps; // 1 sec = 1,000,000 micro sec
	struct timespec s_time, e_time;

	useconds_t elapsed_usec = 0;
	while(is_running) {
		float dt = elapsed_usec / 1000000.0f;
		(void)clock_gettime(CLOCK_MONOTONIC, &s_time);

		__p_scene_update(dt);
		__p_scene_draw();

		(void)clock_gettime(CLOCK_MONOTONIC, &e_time);
		elapsed_usec = (e_time.tv_sec - s_time.tv_sec) * 1000000 // sec to microsec
			+ (e_time.tv_nsec - s_time.tv_nsec) / 1000; // nanosec to microsec

		if (target_usec > elapsed_usec)
			(void)usleep(target_usec - elapsed_usec);
	}
}

int p_engine_init(struct p_game_config* config)
{	
	__p_input_init();
	p_screen_clear();
	__p_cursor_hide();
	
	if (__p_config_load(config) < 0)
		return -1;

	return 0;
}

void p_engine_cleanup(void)
{
	p_cursor_move(0, 0);
	__p_cursor_show();
	__p_input_restore();
	p_screen_clear();
}

void p_engine_quit(void)
{
	is_running = 0;
}

static int __p_config_load(struct p_game_config *config)
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
