#include "main.h"
#include "die.h"
#include "config.h"
#include <assert.h>
#include <stdio.h>

static void set_window_icon(void);

void
init(void)
{
	CFG = config_new(NULL);

	if (!al_init())
		die("Failed to initialize Allegro.");
	
	if (!(display = al_create_display(CFG->display.w, CFG->display.h)))
		die("Failed to create display");

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

	/* timers */
	if (!(timers[TIMER_MAIN] = al_create_timer(1.0 / CFG->display.rate)))
		die("Failed to create main timer");

	/* initialize events */

	if (!(event_queue = al_create_event_queue()))
		die("Failed to create event queue");

	al_register_event_source(event_queue,
		al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	/* last steps */
	al_set_window_title(display, CFG->window.title);
	set_window_icon();

	for (int i = 0; i < TIMER_MAX; i++) {
		al_register_event_source(event_queue,
			al_get_timer_event_source(timers[i]));
	}
}

static void
set_window_icon(void)
{
	const char *icon = CFG->window.icon;

	if (al_filename_exists(icon)) {
		if (!(bitmaps[BITMAP_ICON] = al_load_bitmap(icon)))
			die("Failed to load icon bitmap");

		al_set_display_icon(display, bitmaps[BITMAP_ICON]);
	}
}
