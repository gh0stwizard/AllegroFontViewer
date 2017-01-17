#include "main.h"
#include "config.h"

void
tini(void)
{
	for (int i = 0; i < ARRAY_SIZE(timers); i++) {
		if (timers[i] != NULL)
			al_destroy_timer(timers[i]);
		timers[i] = NULL;
	}

	for (int i = 0; i < FONT_MAX; i++) {
		if (fonts[i] != NULL)
			al_destroy_font(fonts[i]);
		fonts[i] = NULL;
	}

	for (int i = 0; i < ARRAY_SIZE(bitmaps); i++) {
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
