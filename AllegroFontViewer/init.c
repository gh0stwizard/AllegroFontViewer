#include "main.h"
#include "die.h"

void
init(void)
{
	if (!al_init())
		die("Failed to initialize Allegro.");

	if (!(display = al_create_display(SCREEN_W, SCREEN_H)))
		die("Failed to create display");

	al_set_window_title(display, WINDOW_TITLE);


	/* install perepherial */

	if (!al_install_keyboard())
		die("Failed to install keyboard");

	if (!al_install_mouse())
		die("Failed to install mouse");


	/* initialiaze addons */

	if (!al_init_image_addon())
		die("Failed to initialize image addon");

	if (!al_init_font_addon())
		die("Failed to initialize font addon");

	if (!al_init_ttf_addon())
		die("Failed to initialize ttf addon");

	if (!al_init_primitives_addon())
		die("Failed to initialize primitives addon");


	/* allocate resources */

	if (al_filename_exists(ICON_FILE)) {
		if (!(bitmaps[BITMAP_ICON] = al_load_bitmap(ICON_FILE)))
			die("Failed to load icon bitmap");

		al_set_display_icon(display, bitmaps[BITMAP_ICON]);
	}

	if (al_filename_exists(DEFAULT_FONT)) {
		if (!(fonts[FONT_DEFAULT] =
			al_load_font(DEFAULT_FONT, DEFAULT_FONT_SIZE, DEFAULT_FONT_FLAG)))
			die("Failed to load default font");
	}
	else
		die("Failed to find default font");


	/* timers */

	if (!(timers[TIMER_MAIN] = al_create_timer(1.0 / FPS)))
		die("Failed to create main timer");

	/* initialize events */

	if (!(event_queue = al_create_event_queue()))
		die("Failed to create event queue");

	al_register_event_source(event_queue,
		al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	for (int i = 0; i < TIMER_MAX; i++) {
		al_register_event_source(event_queue,
			al_get_timer_event_source(timers[i]));
	}
}
