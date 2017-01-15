#include "die.h"
#include "engine.h"
#include <stdlib.h>

void
die(const char *message)
{
	al_show_native_message_box(NULL, "Fatal error!", "Error", message,
		NULL, ALLEGRO_MESSAGEBOX_ERROR);

	exit(EXIT_FAILURE);
}
