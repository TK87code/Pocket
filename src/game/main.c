#include <stdlib.h> // calloc, free
#include "pocket.h"

#define WORLD_WIDTH 500
#define WORLD_HEIGHT 500

#define VIEW_WIDTH 60
#define VIEW_HEIGHT 20

struct world {
	int map[WORLD_HEIGHT][WORLD_WIDTH];
};

struct camera {
	int x;
	int y;
};

struct island_state {
	struct world world;
	struct camera camera;
};

void island_init(void *user_data)
{
	// TODO: do initialization here like creating a random map	
}

int main(void) 
{
	struct island_state *state = calloc(1, sizeof(struct island_state));
	if (state == NULL)
		return -1;

	state->camera.x = (WORLD_WIDTH / 2) - (VIEW_WIDTH / 2); 
	state->camera.y = (WORLD_HEIGHT / 2) - (VIEW_HEIGHT / 2); 

	struct p_game_config config = {
		.user_data = state,
		.on_init = island_init,
		.target_fps = 60
	};

	if (p_engine_init(&config) < 0) {
		p_engine_cleanup();
		free(state);
		return -2;
	} else {
		p_engine_ignite();
	}

	p_engine_cleanup();
	free(state);

	return 0;
}
