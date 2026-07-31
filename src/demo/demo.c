#include <stdio.h>
#include "pocket.h" // Include the engine API

// 1. Initialization (Called once)
void my_init(void *user_data) {
	// Initialize your game data here
	(void)user_data;
}

// 2. Update logic (Called every frame)
void my_update(void *user_data, float dt) {
	(void)user_data;
	(void)dt;
	struct pkt_event e;
	// Poll input events
	while (pkt_poll_event(&e) == 0) {
		if (e.type == PKT_EVENT_KEY_PRESSED) {
			// Quit if ESC is pressed
			if (e.data.key.key_code == PKT_KEY_ESCAPE) {
				pkt_quit(); 
			}
		}
	}
}

// 3. Draw logic (Called every frame)
void my_draw(void *user_data) {
	// Draw green text at (x:10, y:10)
	(void)user_data;
	pkt_puts(10, 10, PKT_COLOR_GREEN, PKT_COLOR_BLACK, "Hello, Pocket Engine!");
	pkt_puts(10, 11, PKT_COLOR_YELLOW, PKT_COLOR_BLACK, "Press Escape to quit, and start coding your own special game!!");
}

int main(void) {
	struct pkt_config config = pkt_get_default_config();
	config.on_init = my_init; 

	if (pkt_init(&config) < 0) {
		printf("Failed to initialize engine.\n");
		return -1;
	}

	// Register and swap to your scene
	struct pkt_scene scene = {0};
	scene.on_update = my_update;
	scene.on_draw = my_draw;

	pkt_register_scene(0, &scene);
	pkt_swap_scene(0);

	// Start the game loop!
	pkt_ignite();

	// Restore terminal attributes
	pkt_cleanup();

	return 0;
}
