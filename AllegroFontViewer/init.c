#include "main.h"
#include "config.h"
#include <assert.h>
#include <stdio.h>

static void set_window_icon(void);
static void set_timers(void);
static void load_fonts(void);
static void die(const char *message);

void
init(void)
{
	if (!al_init())
		die("Failed to initialize Allegro.");

	/* always read configuration files after allegro initialization */
	CFG = config_new(NULL);
	
	if (!(display = al_create_display(CFG->display.w, CFG->display.h)))
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
	load_fonts();

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

	for (int i = 0; i < TIMER_MAX; i++) {
		timers[i] = al_create_timer(values[i]);
		assert(i >= 0 && timers[i] != NULL);
	}
}

static void
set_display(void)
{
	/*TODO: use the code below :) */

	monitor = calloc(1, sizeof(ALLEGRO_MONITOR_INFO));
	if (al_get_monitor_info(0, monitor) & 0) {
		al_set_new_display_flags(ALLEGRO_FULLSCREEN);
		//al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
		al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_REQUIRE);
		display = al_create_display(monitor->x2 - monitor->x1, 
			monitor->y2 - monitor->y1);
	}
	else {
		al_set_new_display_flags(ALLEGRO_WINDOWED);
		display = al_create_display(CFG->display.w, CFG->display.h);
	}
}

static void
load_fonts(void)
{
	static ALLEGRO_FS_ENTRY *e;
	static FONT_INFO fi;

	fonts[FONT_DEFAULT] = al_create_builtin_font();

	for (int i = FONT_DEFAULT + 1; i < FONT_MAX; i++) {
		fi = CFG->fonts[i];
		e = al_create_fs_entry(fi.file);
		if (al_fs_entry_exists(e))
			fonts[i] = al_load_font(fi.file, fi.size, fi.flags);
		al_destroy_fs_entry(e);
	}
}

static void
die(const char *message)
{
	al_show_native_message_box(NULL, "Fatal error!", "Error", message,
		NULL, ALLEGRO_MESSAGEBOX_ERROR);
	exit(EXIT_FAILURE);
}
