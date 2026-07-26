#include <stddef.h> // NULL
#include "pocket.h"

static struct p_scene registered_scenes[P_MAX_SCENES];

static int current_scene_id = -1; // -1 means no scenes is set currently

int p_scene_register(int scene_id, struct p_scene *scene)
{
	if (scene_id < 0 || scene_id >= P_MAX_SCENES || scene == NULL)
		return -1;

	registered_scenes[scene_id] = *scene;

	return 0;
}

int p_scene_swap(int scene_id)
{
	if (scene_id < 0 || scene_id >= P_MAX_SCENES)
		return -1;

	if (current_scene_id != -1 && registered_scenes[current_scene_id].on_exit)
		registered_scenes[current_scene_id].on_exit(registered_scenes[current_scene_id].user_data);

	if (registered_scenes[scene_id].on_enter)
		registered_scenes[scene_id].on_enter(registered_scenes[current_scene_id].user_data);

	current_scene_id = scene_id;

	return 0;
}

int p_scene_get(void) 
{
	return current_scene_id;
}

// Call the update function of current scene
void __p_scene_update(float dt)
{
	if (current_scene_id != -1 && registered_scenes[current_scene_id].on_update)
		registered_scenes[current_scene_id].on_update(registered_scenes[current_scene_id].user_data, dt);
}

// Call the draw function of current scene
void __p_scene_draw()
{
	if (current_scene_id != -1 && registered_scenes[current_scene_id].on_draw)
		registered_scenes[current_scene_id].on_draw(registered_scenes[current_scene_id].user_data);
}
