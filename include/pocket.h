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
 * @brief  Initialize engine. This disables some terminal attributes
 * under the hood.
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
 * @brief This function stops a game loop.
 */
void p_engine_quit(void);

/**
 * @brief Clean up engine. User should call this before exit, otherwise the user terminal
 * will be broken(some attributes will still be disabled).
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

#define P_DEFAULT_SWIDTH 80
#define P_DEFAULT_SHEIGHT 24

// COLOR CODES
#define P_COLOR_DEFAULT 0 
#define P_COLOR_BLACK 30 
#define P_COLOR_RED 31
#define P_COLOR_GREEN 32
#define P_COLOR_YELLOW 33
#define P_COLOR_BLUE 34
#define P_COLOR_MAGENTA 35
#define P_COLOR_CYAN 36
#define P_COLOR_WHITE 37

/**
 * @brief  Draw character at specified x&y
 *
 * @param  x x position(col)
 * @param  y y position(row)
 * @param  color color codes defined in pocket.h
 * @param  c character to draw
 *
 * @return 0 on success -1 when invalid x, y, or color code passed 
 */
int p_draw_char(int x, int y, int color, char c);

/**
 * @brief  Draw a string at specified position.
 *
 * @param  x x position(col)
 * @param  y y position(row)
 * @param  color color codes defined in pocket.h
 * @param  str String to draw
 *
 * @return 0 on success, -1 when invalid x, y, or color code passed.
 */
int p_draw_str(int x, int y, int color, const char *str);

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
