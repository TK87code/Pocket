#include <stdio.h>
#include "pocket.h"

#define SCENE_TITLE 0
#define SCENE_PLAY 1

struct my_game_data {
	float player_x;
	int score;
};

void title_enter(void *user_data) 
{
	p_screen_clear();
	p_cursor_move(10, 5);
	printf("=== POCKET ENGINE ===\n");
	p_cursor_move(10, 6);
	printf("Press enter to start...\n");
	fflush(stdout);
}

void title_update(void *user_data, float dt) 
{
	int key = p_input_read();
	if (key == '\n' || key == '\r') 
		p_scene_swap(SCENE_PLAY);
}

void play_enter(void *user_data)
{
	p_screen_clear();
	p_cursor_move(10, 5);
	p_fcolor_set(P_COLOR_GREEN);
	printf("Game started! (Press 'q' to quit)\n");
	p_fcolor_set(P_COLOR_DEFAULT);
}

void play_update(void *user_data, float dt)
{
	struct my_game_data *data = (struct my_game_data *)user_data;

	int key = p_input_read();
	if (key == 'd')
		data->player_x += 10.0f * dt;
	else if (key == 'q')
		p_engine_quit();
}

void my_game_init(void *user_data)
{
	struct my_game_data *data = (struct my_game_data *)user_data;

	data->player_x = 0.0f;
	data->score = 0;

	struct p_scene s_title = {
		.user_data = data,
		.on_enter = title_enter,
		.on_update = title_update
	};

	struct p_scene s_play = {
		.user_data = data,
		.on_enter = play_enter,
		.on_update = play_update,
	};

	p_scene_register(SCENE_TITLE, &s_title);
	p_scene_register(SCENE_PLAY, &s_play);

	p_scene_swap(SCENE_TITLE);
}

int main(void) 
{
	struct my_game_data data = {0};

	struct p_game_config config = {
		.user_data = (void *)&data,
		.on_init = my_game_init,
		.target_fps = 90
	};

	if (p_engine_init(&config) == 0) {
		p_engine_ignite();
	}

	p_engine_cleanup();

	return 0;
}
