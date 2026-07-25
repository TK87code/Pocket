#include "pocket.h"
#include "p_input_internal.h"
#include "p_screen_internal.h"

static int is_running = 1; 

void p_engine_ignite(struct p_game_config *config)
{
        if (config->on_init)
		config->on_init();

        while(is_running) {
		if (config->on_update)
			config->on_update();
		if (config->on_draw)
			config->on_draw();
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
