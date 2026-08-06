/*
 * Pocket Public API
 *
 */
#ifndef POCKET_H
#define POCKET_H

#include <stddef.h> // size_t
#include <stdint.h> // uintxx_t

// === Engine CORE API === 
       
/**
 * @brief Color code to specify font and background colors.
 *
 * @note User can use these pkt_color list, or pass ANSI 256 color codes(0 to 255). 
 */
enum pkt_color {
	PKT_COLOR_DEFAULT = -1,
	PKT_COLOR_BLACK = 0, 
	PKT_COLOR_RED = 1,
	PKT_COLOR_GREEN = 2,
	PKT_COLOR_YELLOW = 3,
	PKT_COLOR_BLUE = 4,
	PKT_COLOR_MAGENTA = 5,
	PKT_COLOR_CYAN = 6,
	PKT_COLOR_WHITE = 7,
};

// Character attributes
// [NOTE] This is enabled by ANSI escape sequence, and some attribute is not supported depends on user's terminal.
enum pkt_attr {
	PKT_ATTR_NONE = 0,
	PKT_ATTR_BOLD = (1 << 0), 
	PKT_ATTR_UNDERLINE = (1 << 1), 
	PKT_ATTR_REVERSE = (1 << 2),
	PKT_ATTR_BLINK = (1 << 3),
	PKT_ATTR_FASTBLINK = (1 << 4),
};

struct pkt_config {
	void *user_data; 		 // Pointer to store user data structure. Default-> NULL

	void (*on_init)(void *user_data);//func pointer for user initialization. This must be set.

	int target_fps; 		 // Target fps value that user want to achieve. Default-> 60

	enum pkt_color default_fcolor; 	 // Game's default colors. Default-> fcolor = white : bcolor = black
	enum pkt_color default_bcolor; 	 //  writing functions without color, like pkt_putc() uses these colors 

	int game_cols;			 // Game's logical size. If it's set to 0(default), game logical size is 
	int game_rows;			 // player's terminal size itself. 
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
	PKT_EVENT_RESIZE,
};

struct pkt_event {
	enum pkt_event_type type;
	union {
		struct {
			int key_code;
		} key;
		struct {
			int cols;
			int rows;
		}resize;
	} data;
};

enum pkt_keycode { 
	PKT_KEY_ENTER = 10,
	PKT_KEY_ESCAPE = 27,
	PKT_KEY_SPACE = 32,
	PKT_KEY_UP = 1000, // Avoiding confliction with ASCII
	PKT_KEY_DOWN,
	PKT_KEY_RIGHT,
	PKT_KEY_LEFT,
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

/**
 * @brief Get current terminal size and store it to the pointer passed as parameters. 
 *
 * @param  out_cols A pointer to store current terminal colmmuns.
 * @param  out_rows A pointer to store current terminal rows.
 *
 * @return 0 on success. 
 */
int pkt_get_termsize(int *out_cols, int *out_rows);

/**
 * @brief  Put a 1 byte character at specified x and y coordinates.
 *
 * @param  x X position(columns)
 * @param  y Y position(rows)
 * @param  c A character to put on screen.
 *
 * @return  0 on success. -1 when invalid x, y passed.
 */
int pkt_putc(int x, int y, char c);

/**
 * @brief  Put a 1 byte character at specified x and y coordinates with colors and attributes.
 *
 * @param  x X position(columns)
 * @param  y Y position(rows)
 * @param  fcolor Font color (e.g., PKT_COLOR_RED)
 * @param  bcolor Background color
 * @param  attr Bitmask of character attributes. Can be a bitwise OR of "enum pkt_attr"
 *              values (e.g., PKT_ATTR_BOLD | PKT_ATTR_UNDERLINE). Pass PKT_ATTR_NONE for nomal text.
 * @param  c A character to put on screen
 *
 * @return 0 on success -1 when invalid x, y, or color code passed. 
 */
int pkt_putc_color(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, char c);

/**
 * @brief  Put a string or multi-byte character(UTF-8) at specified x and y coordinates.
 *
 * @param  x X position(columns)
 * @param  y y position(rows)
 * @param  str A string to put on screen
 *
 * @return 0 on success, -1 when invalid x or y passed.
 */
int pkt_puts(int x, int y, const char *str);

/**
 * @brief  Put a string or multi byte character(UTF-8) at specified x and y coordinates
 * 	   with color and attributes.
 *
 * @param  x x position(col)
 * @param  y y position(row)
 * @param  fcolor Font color (e.g., PKT_COLOR_RED) 
 * @param  bcolor Background color
 * @param  attr Bitmask of character attributes. Can be a bitwise OR of "enum pkt_attr"
 *              values (e.g., PKT_ATTR_BOLD | PKT_ATTR_UNDERLINE). Pass PKT_ATTR_NONE for nomal text.
 * @param  str A string to put 
 *
 * @return 0 on success, -1 when invalid parameters passed.
 */
int pkt_puts_color(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, const char *str);

/**
 * @brief Print a formatted string at x and y coordinates. 
 *
 * @param  x X position(columns)
 * @param  y Y position(rows)
 * @param  fmt A formatted string to print
 *
 * @return 0 on success, -1 when invalid x or y passed.
 */
int pkt_printf(int x, int y, const char *fmt, ...);

/**
 * @brief  Print a formatted string at x and y coordinates with color and attributes.
 *
 * @param  x x position (columns)
 * @param  y y positon (rows)
 * @param  fcolor Font color (e.g., PKT_COLOR_RED). You also can pass ANSI 256 color code here.
 * @param  bcolor Background color
 * @param  attr Bitmask of character attributes. Can be a bitwise OR of "enum pkt_attr"
 *              values (e.g., PKT_ATTR_BOLD | PKT_ATTR_UNDERLINE). Pass PKT_ATTR_NONE for nomal text.
 * @param  fmt A formatted string to print
 *
 * @return 0 on success, -1 when invalid parameters passed.
 */
int pkt_printf_color(int x, int y, enum pkt_color fcolor, enum pkt_color bcolor, unsigned int attr, const char *fmt, ...);

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
int pkt_register_scene(int scene_id, const struct pkt_scene *scene);

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

// === Memory Module API ===

// ---------------------------------------------------------
// 1. Arena Allocator (Linear Allocator)
// Purpose: Dynamic allocation of variable-sized memory.
//          Can only be reset entirely at once.
// ---------------------------------------------------------

struct pkt_arena {
	unsigned char *base; // pointer to the top of arena
	size_t size;		// Total size of arena
	size_t offset;		// How much memory used in arena 
};

/**
 * @brief  Allocate memory from the scratch arena.This memory will be automatically reset by the engine at the end of each frames. 
 *
 * @warning  DO NOT free memory reserved by this function.
 * Memory release will be done automatically by the engine at the end of each frames.
 *
 * @param  bytes Size in bytes to allocate.
 *
 * @return  A pointer to allocated memory space, NULL if out of memory.
 */
void *pkt_scratch_alloc(size_t bytes);

/**
 * @brief Initializes a memory arena with a pre-allocated backing buffer. 
 *
 * @param  arena A pointer to the arena structure to initialize.
 * @param  backing_buffer A pointer to the pre-allocated memory block.
 * @param  arena_size Total size of the backing buffer in bytes.
 *
 * @return 0 on success 
 */
int pkt_init_arena(struct pkt_arena *arena, void *backing_buffer, size_t arena_size);

/**
 * @brief  Allocate memory from the arena by advancing its offset.
 *
 * @param  arena A pointer to the arena which gives memory space.
 * @param  bytes the size of memory to allocate. 
 *
 * @return Returns a pointer on success, or NULL if out of memory
 */
void *pkt_arena_alloc(struct pkt_arena *arena, size_t bytes);

/**
 * @brief  Release the memory arena reserved, allowing the memory to be reused. 
 *
 * @param  arena A pointer to the arena to free
 *
 * @return 0 on success
 */
int pkt_reset_arena(struct pkt_arena *arena);

// ---------------------------------------------------------
// 2. Pool Allocator (Free-List)
// Purpose: Allocation of fixed-sized blocks.
//          Allows individual allocation and freeing of elements.
// ---------------------------------------------------------

struct pkt_pool_node {
	struct pkt_pool_node *next;
};

struct pkt_pool_manager {
	unsigned char *base; 		// Pointer to the memory buffer
	size_t block_size;		// Size of a single element
	size_t max_elems;		// Max number of elements
	struct pkt_pool_node *head;	// Head of the free-list
};

/**
 * @brief  Initialize a memory pool with a pre-allocated buffer and block size.
 * 	   This build a free-list of available blocks internally.
 *
 * @param  manager A pointer to the pool manager structure to initialize.
 * @param  backing_buffer A pre-allocated memory block.
 * @param  block_size The size of a single element (e.g., sizeof(struct chunk)).
 * @param  max_elems Maximum number of elements the buffer can hold.
 *
 * @return 
 */
int pkt_pool_init(struct pkt_pool_manager *manager, void *backing_buffer, size_t block_size, size_t max_elems);

/**
 * @brief  Allocate a single memory block from the pool's free-list.
 *
 * @param  manager A pointer to the memory pool.
 *
 * @return A pointer to the a;;ocated block, or NULL if the pool is empty. 
 */
void *pkt_pool_alloc(struct pkt_pool_manager *manager);

/**
 * @brief  Free a memory block back to the pool by prepending it to the free-list.
 *
 * @param  manager A pointer to the memory pool.
 * @param  ptr A pointer to the memory block to free.
 *
 * @return 0 on success, -1 if the pointer is invalid. 
 */
int pkt_pool_free(struct pkt_pool_manager *manager, void *ptr);

// === Log Module API ===

enum pkt_log_level {
	PKT_LOG_INFO,
	PKT_LOG_WARN,
	PKT_LOG_ERROR
};

#ifdef PKT_DEBUG
int __pkt_log_out(enum pkt_log_level level, const char *fmt, ...);
#define PKT_LOG(level, ...) __pkt_log_out(level, __VA_ARGS__)

#else
#define PKT_LOG(level, ...) ((void)0)

#endif //#ifdef PKT_DEBUG

#endif //#ifndef POCKET_H
