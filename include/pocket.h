/*
 * Pocket Public API
 *
 */
#ifndef POCKET_H
#define POCKET_H

#include <stddef.h> // size_t

// === Engine CORE API === 

struct pkt_config {
	void *user_data; // Pointer to store user data structure
	void (*on_init)(void *user_data); //func pointer for user initialization
	int target_fps; // Target fps value that user want to achieve
	int screen_col; // number of columns on screen
	int screen_row; // number of rows on screen
};

/**
 * @brief  Get a default configuration structure initialized with safe values.
 * Use this configuration and change parameters you want to customize.
 *
 * @return struct pkt_config with default parameters. 
 */
struct pkt_config pkt_get_default_config(void);

/**
 * @brief  Initialize engine. This disables some terminal attributes
 * under the hood.
 * 
 * @param  config structure passed by user.
 *
 * @return  0 on success. 
 * ERROR-> -1: Failed to initialize terminal -2: invalid config.
 */
int pkt_init(struct pkt_config *config);

/**
 * @brief  Start the game loop
 * 
 * @return  0 on success.
 */
int pkt_ignite(void);

/**
 * @brief  This function stops a game loop.
 * 
 * @return  0 on success.
 */
int pkt_quit(void);

/**
 * @brief  Clean up engine and restore terminal attributes.
 * 
 * @warning MUST call this function before exiting program, otherwise the user terminal screen
 * will be broken (some attributes will still be disabled).
 *
 * @return  0 on success.
 */
int pkt_cleanup(void);

// === Event Module API ===

enum pkt_event_type{
	PKT_EVENT_QUIT,
	PKT_EVENT_KEY_PRESSED,
};

struct pkt_event {
	enum pkt_event_type type;
	union {
		struct {
			int key_code;
		} key;
	} data;
};

/**
 * @brief  Poll event from the engine and store the memory address to the oldest event
 * to the event pointer provided. 
 *
 * @warning  DO NOT use standard C input functions (like getchar() or scanf())
 * inside your scene callbacks. They will blocl the engine's game loop and breal the
 * frame rate. Always use this event poller to get user inputs.
 *
 * @param  event A pointer to an event struct.
 *
 * @return  0 on success, -1 if no event is in queue.
 */
int pkt_poll_event(struct pkt_event *event);

// === Terminal Module API === 

// COLOR CODES
#define PKT_COLOR_DEFAULT 0 
#define PKT_COLOR_BLACK 30 
#define PKT_COLOR_RED 31
#define PKT_COLOR_GREEN 32
#define PKT_COLOR_YELLOW 33
#define PKT_COLOR_BLUE 34
#define PKT_COLOR_MAGENTA 35
#define PKT_COLOR_CYAN 36
#define PKT_COLOR_WHITE 37

/**
 * @brief  Put a character at specified x and y coordinates
 *
 * @param  x x position(col)
 * @param  y y position(row)
 * @param  color color codes defined in pocket.h
 * @param  c character to draw
 *
 * @return 0 on success -1 when invalid x, y, or color code passed 
 */
int pkt_putch(int x, int y, int color, char c);

/**
 * @brief  Put a string at specified x and y coordinates
 *
 * @param  x x position(col)
 * @param  y y position(row)
 * @param  color color codes defined in pocket.h
 * @param  str String to draw
 *
 * @return 0 on success, -1 when invalid x, y, or color code passed.
 */
int pkt_putstr(int x, int y, int color, const char *str);

// === Scene Module API ===

struct pkt_scene {
	void *user_data;
	// parameters: user data and deltatime
	void (*on_enter)(void *user_data); // The user defined func which is called once when enter
	void (*on_update)(void *user_data, float dt); // The user defined func called every frame
	void (*on_draw)(void *user_data); // The user defined func called every frame
	void (*on_exit)(void *user_data); // The user defined func called once when exit
};

/**
 * @brief Register user-defined scene. Scene ID must be 0 or greater and
 *        does not exceed PKT_MAX_SCENES
 *
 * @param  scene_id scene id (0 < X < PKT_MAX_SCENES)
 * @param  scene a pointer to struct pkt_scene
 *
 * @return return 0 on success. -1 on failure 
 */
int pkt_register_scene(int scene_id, struct pkt_scene *scene);

/**
 * @brief Swap to the registered scene specified by ID  
 *
 * @param  next_scene_id ID of the next scene which was used when registered
 *
 * @return 0 on success ERROR-> -1: Invalid scene id
 */
int pkt_swap_scene(int next_scene_id);

/**
 * @brief Get current scene
 *
 * @return Returns scene ID. -1 means no scenes registered yet 
 */
int pkt_get_scene(void);

// === Memory Arena API ===

struct pkt_arena {
	unsigned char *base; // pointer to the top of arena
	size_t size;		// Total size of arena
	size_t offset;		// How much memory used in arena 
};

/**
 * @brief  Reserve memory space in Frame Arena which will be cleaned up each frame by the engine,
 * so user does not have to clear.
 *
 * @warning  DO NOT free memory reserved by this function. It will cause segmentation fault
 * and crash the program. Memory release (re-setting memory offset) will be done end of the each
 * frame by the engine automatically.
 *
 * @param  bytes Size in bytes to reserve.
 *
 * @return  A pointer to reserved memory space, NULL if out of memory.
 */
void *pkt_reserve_scratch_memory(size_t bytes);

/**
 * @brief Initializes a memory arena with a pre-allocated backing buffer 
 *
 * @param  arena A pointer to the arena structure to initialize
 * @param  backing_buffer A pointer to the pre-allocated memory block
 * @param  arena_size Total size of the backing buffer in bytes
 *
 * @return 0 on success 
 */
int pkt_init_memory_arena(struct pkt_arena *arena, void *backing_buffer, size_t arena_size);

/**
 * @brief  Allocate memory from arena and returns the pointer to the designated memory
 *
 * @param  arena pointer to the arena which gives memory space  
 * @param  bytes the size of memory that user wants for this allocation
 *
 * @return Returns a pointer on success, or NULL if out of memory
 */
void *pkt_reserve_memory(struct pkt_arena *arena, size_t bytes);

/**
 * @brief  Release the memory arena reserved, allowing the memory to be reused. 
 *
 * @param  arena A pointer to the arena to free
 *
 * @return 0 on success
 */
int pkt_release_memory(struct pkt_arena *arena);

#endif 
