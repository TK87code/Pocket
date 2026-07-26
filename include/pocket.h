/*
 * Pocket Public API
 *
 */
#ifndef P_POCKET_H
#define P_POCKET_H

// === Engine core API === 

struct p_game_config {
	void *user_data; // Pointer to store user data structure
	void (*on_init)(void *user_data); //func pointer for user initialization
	int target_fps;
};

/**
 * @brief  Initialize engine
 * 
 * @param config structure passed by user.
 *
 * @return 0 on success. ERROR-> -1 invalid config structure passed.
 */
int p_engine_init(struct p_game_config *config);

/**
 * @brief  Start the game loop
 *
 */
void p_engine_ignite(void);

/**
 * @brief This function stops a game loop. User must call this to quit application normally.
 */
void p_engine_quit(void);

/**
 * @brief Clean up engine. User should call this before exit. 
 */
void p_engine_cleanup(void);

// === Input Module API === 

/**
 * @brief  Read 1 character from a key board and return the key code as an int.
 *
 * @return  return the character read as an int
 */
int p_input_read(void);

// === Screen Module API === 

// COLOR CODES
#define P_COLOR_DEFAULT 0 
#define P_COLOR_RED 31
#define P_COLOR_GREEN 32
#define P_COLOR_BLUE 34

/**
 * @brief  Clear screen
 */
void p_screen_clear(void);

/**
 * @brief  Move cursor to specified x and y
 *
 * @param  x x(COL) coordinate 
 * @param  y y(ROW) coordinate
 */
void p_cursor_move(int x, int y);

/**
 * @brief  Change color of characters
 *
 * @param  color_code Color code difined in p_screen.h
 */
void p_fcolor_set(int color_code);

// === Scene Module API ===

#define P_MAX_SCENES 16 // Max scene slot available for user to register

struct p_scene {
	void *user_data;
	// parameters: user data and deltatime
	void (*on_enter)(void *user_data); // The user defined func which is called once when enter
	void (*on_update)(void *user_data, float dt); // The user defined func called every frame
	void (*on_draw)(void *user_data); // The user defined func called every frame
	void (*on_exit)(void *user_data); // The user defined func called once when exit
};

/**
 * @brief Register user defined scene. scene id must be 0 or greater and
 *        does not exeed P_MAX_SCENES
 *
 * @param  scene_id scene id (0 < X < P_MAX_SCENES)
 * @param  scene a pointer to struct p_scene
 *
 * @return return 0 on success. -1 on failure 
 */
int p_scene_register(int scene_id, struct p_scene *scene);

/**
 * @brief Swap to the registered scene specified by ID  
 *
 * @param  next_scene_id ID of the next scene which was used when registerd
 *
 * @return 0 on success ERROR-> -1: Invalid scene id
 */
int p_scene_swap(int next_scene_id);

/**
 * @brief Get current scene
 *
 * @return Returns scene ID. -1 means no scenes registered yet 
 */
int p_scene_get(void);

#endif 
