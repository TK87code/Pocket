/*
 * Pocket Public API
 *
 */
#ifndef P_POCKET_H
#define P_POCKET_H

// === Engine core API === 

struct p_game_config {
	void (*on_init)(void);
	void (*on_update)(void);
	void (*on_draw)(void);
};

/**
 * @brief  Initialize engine
 */
void p_engine_init(void);

/**
 * @brief  Start the game loop
 *
 * @param  p_game_config config structure defined in Pocket.h
 */
void p_engine_ignite(struct p_game_config *config);

/**
 * @brief  
 */
void p_engine_cleanup(void);


/**
 * @brief This function stops a game loop. User must call this to quit application normally.
 */
void p_engine_quit(void);

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

#endif 
