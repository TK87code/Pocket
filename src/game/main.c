#include <stdlib.h> // calloc, free
#include "pocket.h"

struct my_game {
	int x;
	int y;
};

void my_enter(void *user_data) 
{
	struct my_game *game = user_data;
	game->x = 10;
	game->y = 10;
}

void my_update(void *user_data, float dt)
{
	(void)dt;
	struct my_game *game = user_data;
	int key = p_input_read();

	if (key == 'w')
		game->y--;
	if (key == 's')
		game->y++;
	if (key == 'a')
		game->x--;
	if (key == 'd')
		game->x++;
	if (key == 'q')
		p_engine_quit();
}

void my_draw(void *user_data)
{
	struct my_game *game = user_data;

	p_draw_str(2, 2, P_COLOR_GREEN, "test");
	p_draw_str(2, 3, P_COLOR_DEFAULT, "TEST");
	
	p_draw_char(game->x, game->y, P_COLOR_RED, '@');
}

void engine_ready(void *user_data) {
	(void)user_data;
	p_scene_swap(0);
}

int main(void) 
{
	struct my_game game = {0};

	struct p_scene test_scene = {
		.user_data = &game,
		.on_enter = my_enter,
		.on_update = my_update,
		.on_draw = my_draw,
		.on_exit = NULL
	};

	p_scene_register(0, &test_scene);

	struct p_game_config config = {
		.user_data = NULL,
		.on_init = engine_ready,
		.target_fps = 60
	};

	if (p_engine_init(&config) < 0) {
		p_engine_cleanup();
		return -1;
	}

	p_engine_ignite();
	p_engine_cleanup();

	return 0;
}
