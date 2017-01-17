#include "main.h"
#include "die.h"
#include "config.h"
#include <assert.h>
#include <stdio.h>

static void set_window_title(void);
static void set_window_icon(void);
static void set_default_font(void);

void
init(void)
{
	if (!config_init(NULL))
		die("Failed to load configuration files");

	if (!al_init())
		die("Failed to initialize Allegro.");

	{
		int w, h;
		config_get_value("display", "width", CONFIG_AS_INT, &w);
		config_get_value("display", "height", CONFIG_AS_INT, &h);
		if (!(display = al_create_display(w, h)))
			die("Failed to create display");
	}

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
	{
		float fps;
		config_get_value("display", "rate", CONFIG_AS_FLOAT, &fps);
		if (!(timers[TIMER_MAIN] = al_create_timer(1.0 / fps)))
			die("Failed to create main timer");
	}

	/* initialize events */

	if (!(event_queue = al_create_event_queue()))
		die("Failed to create event queue");

	al_register_event_source(event_queue,
		al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	/* last steps */
	set_window_title();
	set_window_icon();
	set_default_font();

	for (int i = 0; i < TIMER_MAX; i++) {
		al_register_event_source(event_queue,
			al_get_timer_event_source(timers[i]));
	}
}

inline static void
set_window_title(void)
{
	al_set_window_title(display, config_get_value("window", "title", 0, NULL));
}

static void
set_window_icon(void)
{
	const char *icon = config_get_value("window", "icon", 0, NULL);

	if (al_filename_exists(icon)) {
		if (!(bitmaps[BITMAP_ICON] = al_load_bitmap(icon)))
			die("Failed to load icon bitmap");

		al_set_display_icon(display, bitmaps[BITMAP_ICON]);
	}
}

static void
set_default_font(void)
{
#define SECTION "font:default"
	int size;
	const char *file = config_get_value(SECTION, "file", 0, NULL);
	config_get_value(SECTION, "file", CONFIG_AS_INT, &size);

	if (al_filename_exists(file)) {
		if (!(fonts[FONT_DEFAULT] = al_load_font(file, size, 0)))
			die("Failed to load default font");
	}
	else
		die("Failed to find default font");

#undef SECTION
}
