#include "display.h"
#include <assert.h>


const int WINDOWED_FLAGS = ALLEGRO_WINDOWED 
	| ALLEGRO_RESIZABLE 
	| ALLEGRO_GENERATE_EXPOSE_EVENTS;

const int ADAPTER = 0; /* TODO multiplie adapters support? */


ALLEGRO_DISPLAY *
create_display(DISPLAY_INFO *di)
{
	ALLEGRO_MONITOR_INFO monitor;
	ALLEGRO_DISPLAY *d = NULL;
	static int max_w, max_h;

	assert(al_get_monitor_info(ADAPTER, &monitor));

	max_w = monitor.x2 - monitor.x1;
	max_h = monitor.y2 - monitor.y1;

	if (di->fullscreen) {
		al_set_new_display_flags((di->fswindowed)
			? ALLEGRO_FULLSCREEN_WINDOW
			: ALLEGRO_FULLSCREEN);
		al_set_new_display_option(ALLEGRO_VSYNC, (di->vsync) ? 1 : 2, 
			ALLEGRO_REQUIRE);
		al_set_new_display_refresh_rate(60);
		d = al_create_display(max_w, max_h);
	}
	else {
		al_set_new_display_flags(WINDOWED_FLAGS);
		al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_REQUIRE);
		al_set_new_display_refresh_rate(60);
		d = al_create_display(di->w, di->h);
		al_set_window_constraints(d, di->w, di->h, max_w, max_h);
	}

	return d;
}
