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

	struct pkt_event e;
	while (pkt_poll_event(&e) != -1) {
		if (e.type == PKT_EVENT_KEY_PRESSED)
			if (e.data.key.key_code == 'q' || e.data.key.key_code == 'Q')
				pkt_quit();
	}
}

void my_scene_draw(void *user_data)
{
	struct game_state *state = (struct game_state *)user_data;

	char *text_buffer = (char *)pkt_reserve_scratch_memory(64);

	if (text_buffer != NULL) {
		pkt_printf(5, 5, PKT_COLOR_CYAN, PKT_COLOR_BLACK, "Arena & print test - Frame count: %d", state->frame_count);
		pkt_puts(5, 7, PKT_COLOR_YELLOW, PKT_COLOR_RED, "Press Ctrl + C to exit");
		pkt_puts(5, 9, PKT_COLOR_YELLOW, PKT_COLOR_RED, "テスト！");
	}

}

void on_game_init(void *user_data)
{
	static struct pkt_scene main_scene = {0};
	main_scene.user_data = user_data;
	main_scene.on_update = my_scene_update;
	main_scene.on_draw = my_scene_draw;

	pkt_register_scene(0, &main_scene);
	pkt_swap_scene(0);
	
}

int main(void) 
{
	struct game_state state = {0};
	state.frame_count = 0;

	struct pkt_config config = pkt_get_default_config();
	config.user_data = &state;
	config.on_init = on_game_init;
	config.target_fps = 120;
	config.default_bcolor = PKT_COLOR_MAGENTA;

	if (pkt_init(&config) < 0) {
		pkt_cleanup();
		return -1;
	}

	pkt_ignite();
	pkt_cleanup();

	return 0;
}
