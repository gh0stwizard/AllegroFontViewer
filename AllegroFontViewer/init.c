#include "main.h"
#include "config.h"
#include <assert.h>
#include <stdio.h>


static void set_window_icon(void);
static void set_timers(void);
static void die(const char *message);
static ALLEGRO_DISPLAY * 
create_display(ALLEGRO_MONITOR_INFO *mon, int w, int h);

void
init(void)
{
	if (!al_init())
		die("Failed to initialize Allegro.");

	/* always read configuration files after allegro initialization */
	CFG = config_new(NULL);

	if (!(monitor = al_malloc(sizeof(ALLEGRO_MONITOR_INFO))))
		die("Failed to allocate monitor structure.");
	
	if (!(display = create_display(monitor, CFG->display.w, CFG->display.h)))
		die("Failed to create display.");

	CFG->display.w = al_get_display_width(display);
	CFG->display.h = al_get_display_height(display);


	/* install perepherial */

	if (!al_install_keyboard())
		die("Failed to install keyboard.");

	if (!al_install_mouse())
		die("Failed to install mouse.");

	/* initialiaze addons */

	if (!al_init_image_addon())
		die("Failed to initialize image addon.");

	if (!al_init_font_addon())
		die("Failed to initialize font addon.");

	if (!al_init_ttf_addon())
		die("Failed to initialize ttf addon.");

	if (!al_init_primitives_addon())
		die("Failed to initialize primitives addon.");

	/* allocate resources */

	set_timers();

	/* initialize events */

	if (!(event_queue = al_create_event_queue()))
		die("Failed to create event queue.");

	al_register_event_source(event_queue,
		al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_mouse_event_source());

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
			die("Failed to load icon bitmap.");

		al_set_display_icon(display, bitmaps[BITMAP_ICON]);
	}
}


static void
set_timers(void)
{
	float values[TIMER_MAX];
	values[0] = (1.0 / CFG->display.rate);
	values[1] = (CFG->keyboard.repeatrate / 1000.0);
	values[2] = CFG->typer.blink_period;

	for (int i = 0; i < TIMER_MAX; i++) {
		timers[i] = al_create_timer(values[i]);
		assert(i >= 0 && timers[i] != NULL);
	}
}


static ALLEGRO_DISPLAY *
create_display(ALLEGRO_MONITOR_INFO *mon, int w, int h)
{
	ALLEGRO_DISPLAY *d;

	if (mon == NULL)
		return NULL;

	const int adapter = 0; /* TODO multiplie adapters support? */

	if (CFG->display.fullscreen && al_get_monitor_info(adapter, mon)) {
		al_set_new_display_flags((CFG->display.fswindowed) 
			? ALLEGRO_FULLSCREEN_WINDOW
			: ALLEGRO_FULLSCREEN);
		al_set_new_display_option(ALLEGRO_VSYNC, (CFG->display.vsync) ? 1 : 2, 
			ALLEGRO_REQUIRE);
		d = al_create_display(mon->x2 - mon->x1, mon->y2 - mon->y1);
	}
	else {
		al_set_new_display_flags(ALLEGRO_WINDOWED);
		d = al_create_display(w, h);
	}

	return d;
}


static void
die(const char *message)
{
	al_show_native_message_box(NULL, "Fatal error!", "Error", message,
		NULL, ALLEGRO_MESSAGEBOX_ERROR);
	exit(EXIT_FAILURE);
}
