#include <stdio.h>
#include <termios.h>
#include <unistd.h> // STDIN_FILENO
#include "pocket.h"
#include "p_input_internal.h"

static struct termios original_term;

void __p_input_init(void) 
{
	tcgetattr(STDIN_FILENO, &original_term); // store user terminal attribute to restore later.
	struct termios new_term = original_term;
	new_term.c_lflag &= ~(ECHO | ICANON); // Disable echo and ICANON(waiting untill user hit enter)
	new_term.c_cc[VMIN] = 0; // wait at least 0 characters
	new_term.c_cc[VTIME] = 0; // get realtime input 
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term); // Set terminal attribute immediately(TCSANOW)
}

void __p_input_restore(void) 
{
	tcsetattr(STDIN_FILENO, TCSANOW, &original_term); // Restore original attribute 
}

int p_input_read(void) 
{
	unsigned char c;
	// order OS to read 1 byte directly from file descriptor
	if (read(STDIN_FILENO, &c, 1) == 1)
		return c; // Key has pressed 

	return -1; // Key has not pressed 
}


