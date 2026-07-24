#ifndef P_INPUT_H
#define P_INPUT_H

/**
 * @brief  Initialize Pocket console game engine. This disable Canonical mode and echo in terminal setting.
 */
void p_input_init(void);

/**
 * @brief  Terminate Pocket console game engin. This reset terminal settings.
 */
void p_input_restore(void);

/**
 * @brief  Read 1 character from a key board and return the key code as an int.
 *
 * @return  return the character read as an int
 */
int p_input_read(void);

#endif
