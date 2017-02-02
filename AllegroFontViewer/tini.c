#include "main.h"


void
tini(void)
{
	/* TODO: monitor, camera */

	for (size_t i = 0; i < ARRAY_SIZE(timers); i++) {
		if (timers[i] != NULL)
			al_destroy_timer(timers[i]);
		timers[i] = NULL;
	}

	for (size_t i = 0; i < ARRAY_SIZE(bitmaps); i++) {
		if (bitmaps[i] != NULL)
			al_destroy_bitmap(bitmaps[i]);
		bitmaps[i] = NULL;
	}

	if (event_queue != NULL)
		al_destroy_event_queue(event_queue);

	if (display != NULL)
		al_destroy_display(display);

	config_destroy(CFG);
}
