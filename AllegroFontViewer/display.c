#include "display.h"
#include <assert.h>

#if defined(_DEBUG)
#include <stdio.h>
#endif


const int WINDOWED_FLAGS = ALLEGRO_WINDOWED | ALLEGRO_RESIZABLE
    | ALLEGRO_GENERATE_EXPOSE_EVENTS;

const int ADAPTER = 0; /* TODO multiple adapters support? */


ALLEGRO_DISPLAY *
create_display(DISPLAY_INFO *di)
{
	ALLEGRO_DISPLAY *d = NULL;
	int w, h, max_w, max_h;

	assert(get_max_resolution(&max_w, &max_h));

	al_reset_new_display_options();

	if (di->fullscreen) {
		if (di->fswindowed) {
			al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
		}
		else {
			al_set_new_display_flags(ALLEGRO_FULLSCREEN);

			/**
			 * FIXME: create_display crashes under linux if framerate >= 60.0.
			 * When framerate 59.0 all is OK.
			 */
#ifdef __linux__
			if (di->framerate >= 1.0)
				al_set_new_display_refresh_rate(di->framerate - 1.0);
#else
				al_set_new_display_refresh_rate(di->framerate);
#endif

			if (di->vsync)
				al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
			else
				al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_SUGGEST);
		}

#if defined(_DEBUG)
		fprintf(stderr, "fullscreen w: %d h: %d, windowed: %s fps: %.2f\n",
			max_w, max_h, di->fswindowed ? "Yes" : "No", di->framerate);
#endif

		d = al_create_display(max_w, max_h);
	}
	else {
		int flags = WINDOWED_FLAGS;

		if (di->maximized) {
			flags |= ALLEGRO_MAXIMIZED;
		}
		else {
			if ((di->x != INT_MAX) && (di->y != INT_MAX))
				al_set_new_window_position(di->x, di->y);
		}

		al_set_new_display_flags(flags);

		if ((di->window_w > 0) && (di->window_w < max_w))
			w = di->window_w;
		else
			w = di->w;

		if ((di->window_h > 0) && (di->window_h < max_h))
			h = di->window_h;
		else
			h = di->h;

#if defined(_DEBUG)
		fprintf(stderr, "window w: %d h: %d maximize: %s\n",
				w, h, di->maximized ? "Yes" : "No");
#endif

		d = al_create_display(w, h);

		if ((d != NULL) && (!di->maximized)) /* need a patch */
			al_set_window_constraints(d, di->w, di->h, max_w, max_h);
	}

	return d;
}


bool
get_max_resolution(int *max_w, int *max_h)
{
	static ALLEGRO_MONITOR_INFO monitor;

	if (al_get_monitor_info(ADAPTER, &monitor)) {
		*max_w = monitor.x2 - monitor.x1;
		*max_h = monitor.y2 - monitor.y1;
		return true;
	}
	else
		return false;
}

