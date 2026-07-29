#include <stddef.h> // NULL
#include "pocket.h"

#define PKT_MAX_SCENES 16 

static struct pkt_scene registered_scenes[PKT_MAX_SCENES];

static int current_scene_id = -1; // -1 means no scenes is set currently

int __pkt_scene_register(int scene_id, struct pkt_scene *scene)
{
	if (scene_id < 0 || scene_id >= PKT_MAX_SCENES || scene == NULL)
		return -1;

	registered_scenes[scene_id] = *scene;

	return 0;
}

int __pkt_scene_swap(int next_scene_id)
{
	if (next_scene_id < 0 || next_scene_id >= PKT_MAX_SCENES)
		return -1;

	if (current_scene_id != -1 && registered_scenes[current_scene_id].on_exit)
		registered_scenes[current_scene_id].on_exit(registered_scenes[current_scene_id].user_data);

	if (registered_scenes[next_scene_id].on_enter)
		registered_scenes[next_scene_id].on_enter(registered_scenes[next_scene_id].user_data);

	current_scene_id = next_scene_id;

	return 0;
}

int __pkt_scene_get(void) 
{
	return current_scene_id;
}

// Call the update function of current scene
void __pkt_scene_update(float dt)
{
	if (current_scene_id != -1 && registered_scenes[current_scene_id].on_update)
		registered_scenes[current_scene_id].on_update(registered_scenes[current_scene_id].user_data, dt);
}

// Call the draw function of current scene
void __pkt_scene_draw()
{
	if (current_scene_id != -1 && registered_scenes[current_scene_id].on_draw)
		registered_scenes[current_scene_id].on_draw(registered_scenes[current_scene_id].user_data);
}
