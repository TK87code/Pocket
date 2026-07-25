#include <stdio.h>
#include "../../include/pocket.h"

void my_init(void)
{
	printf("Rougue demo Init,,,\n");
}

void my_update(void)
{

}

void my_draw(void)
{

}

int main(void) 
{
	p_engine_init();

	struct p_game_config config = {my_init, my_update, my_draw};
	p_engine_ignite(&config);

	p_engine_cleanup();

	return 0;
}
