#include <stdio.h>
#include "pocket.h" // Include the engine API

struct data {
	int is_resized;
};

// 1. Initialization (Called once)
void my_init(void *user_data) {
	struct data *d = (struct data *)user_data;
	d->is_resized = 0;
}

// 2. Update logic (Called every frame)
void my_update(void *user_data, float dt) {
	struct data *d = (struct data *)user_data;
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
		
		if (e.type == PKT_EVENT_RESIZE) {
			d->is_resized = 1;
		}
	}
}

// 3. Draw logic (Called every frame)
void my_draw(void *user_data) {
	// Draw green text at (x:10, y:10)
	struct data *d = (struct data *)user_data;
	pkt_puts_color(10, 10, PKT_COLOR_GREEN, PKT_COLOR_BLACK, PKT_ATTR_BLINK, "Hello, Pocket Engine!");
	pkt_puts_color(10, 11, PKT_COLOR_YELLOW, PKT_COLOR_BLACK, PKT_ATTR_FASTBLINK, "Press Escape to quit, and start coding your own special game!!");
	pkt_puts(10, 12, "This supports UTF-8 あｱ†");
	if (d->is_resized) { 	
		int col, row;
		pkt_get_termsize(&col, &row);
		pkt_printf(10, 13, "Resize detected!new terminal size %d : %d", col, row);
	}
}

int main(void) {
	struct data d = {0};
	struct pkt_config config = pkt_get_default_config();
	config.on_init = my_init; 
	config.user_data = &d;

	if (pkt_init(&config) < 0) {
		printf("Failed to initialize engine.\n");
		return -1;
	}

	// Register and swap to your scene
	struct pkt_scene scene = {0};
	scene.user_data = &d;
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
