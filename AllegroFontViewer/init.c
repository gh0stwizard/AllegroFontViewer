#include "main.h"
#include "config.h"
#include "display.h"
#include <assert.h>


static void set_window_icon(void);
static void set_timers(void);
static void die(const char *message);


void
init(void)
{
	if (!al_init())
		die("Failed to initialize Allegro.");

	/* always read configuration files after allegro initialization */
	CFG = config_new(NULL);
	
	DISPLAY_INFO di = {
		.fullscreen = CFG->display.fullscreen,
		.fswindowed = CFG->display.fswindowed,
		.vsync = CFG->display.vsync,
		.w = CFG->display.w,
		.h = CFG->display.h
	};

	if (!(display = create_display(&di)))
		die("Failed to create display.");


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

	if ((icon != NULL) && al_filename_exists(icon)) {
		if (!(bitmaps[BITMAP_ICON] = al_load_bitmap(icon)))
			die("Failed to load icon bitmap.");

		al_set_display_icon(display, bitmaps[BITMAP_ICON]);
	}
}


static void
set_timers(void)
{
	float values[TIMER_MAX];
	values[TIMER_STATUS]	= CFG->status.timeout;
	values[TIMER_KEYBOARD]	= (CFG->keyboard.repeatrate / 1000.0);
	values[TIMER_BLINK]		= CFG->typer.blink_period;

	for (int i = 0; i < TIMER_MAX; i++) {
		if (values[i] > 0.000000) {
			timers[i] = al_create_timer(values[i]);
			assert(timers[i] != NULL);
		}
		else
			timers[i] = NULL;
	}
}


static void
die(const char *message)
{
	al_show_native_message_box(NULL, "Fatal error!", "Error", message,
		NULL, ALLEGRO_MESSAGEBOX_ERROR);
	exit(EXIT_FAILURE);
}
