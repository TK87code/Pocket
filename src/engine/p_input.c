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
	tcsetattr(STDIN_FILENO, TCSANOW, &new_term); // Set terminal attribute immediately(TCSANOW)
}

void __p_input_restore(void) 
{
	tcsetattr(STDIN_FILENO, TCSANOW, &original_term); // Restore original attribute 
}

int p_input_read(void) 
{
	return getchar();
}


