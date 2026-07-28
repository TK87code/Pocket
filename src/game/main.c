#include <stdio.h> 
#include "pocket.h"

struct game_state {
	int frame_count;
};

void my_scene_update(void *user_data, float dt)
{
	(void)dt;
	struct game_state *state = (struct game_state *)user_data;
	state->frame_count++;
}

void my_scene_draw(void *user_data)
{
	struct game_state *state = (struct game_state *)user_data;

	char *text_buffer = (char *)pocket_reserve_frame_arena(64);

	if (text_buffer != NULL) {
		snprintf(text_buffer, 64, "Arena test - Frame count: %d", state->frame_count);
		p_screen_putstr(5, 5, P_COLOR_CYAN, text_buffer);
		p_screen_putstr(5, 7, P_COLOR_YELLOW, "Press Ctrl + C to exit");
	}

}

void on_game_init(void *user_data)
{
	static struct p_scene main_scene = {0};
	main_scene.user_data = user_data;
	main_scene.on_update = my_scene_update;
	main_scene.on_draw = my_scene_draw;

	p_scene_register(0, &main_scene);
	p_scene_swap(0);
}

int main(void) 
{
	struct game_state state = {0};
	state.frame_count = 0;

	struct p_game_config config = {
		.user_data = &state,
		.on_init = on_game_init,
		.target_fps = 120
	};

	if (pocket_init(&config) < 0) {
		pocket_cleanup();
		return -1;
	}

	pocket_ignite();
	pocket_cleanup();

	return 0;
}
