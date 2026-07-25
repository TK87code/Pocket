#include <time.h>    // clock_gettime
#include <unistd.h>  // usleep
#include "pocket.h"
#include "p_input_internal.h"
#include "p_screen_internal.h"
#include "p_scene_internal.h"

#define DEFAULT_TARGET_FPS 60

static int is_running = 1; 
static int target_fps;

void p_engine_ignite(struct p_game_config *config)
{
        if (config->on_init)
		config->on_init();

	if (config->target_fps <= 0)
		target_fps = DEFAULT_TARGET_FPS;
	else
		target_fps = config->target_fps;

	useconds_t target_usec = 1000000 / target_fps; // 1 sec = 1,000,000 micro sec
	struct timespec s_time, e_time;

        while(is_running) {
		(void)clock_gettime(CLOCK_MONOTONIC, &s_time);

		__p_scene_update();
		__p_scene_draw();

		(void)clock_gettime(CLOCK_MONOTONIC, &e_time);
		useconds_t elapsed_usec = (e_time.tv_sec - s_time.tv_sec) * 1000000 // sec to microsec
			            + (e_time.tv_nsec - s_time.tv_nsec) / 1000; // nanosec to microsec

		if (target_usec > elapsed_usec)
			(void)usleep(target_usec - elapsed_usec);
	}
}

void p_engine_init(void)
{
	__p_input_init();
	p_screen_clear();
	__p_cursor_hide();
}

void p_engine_cleanup(void)
{
	p_screen_clear();
	__p_cursor_show();
	__p_input_restore();
}

void p_engine_quit(void)
{
	is_running = 0;
}
