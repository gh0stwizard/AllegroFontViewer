#include "loop.h"
#include "main.h"
#include "colors.h"
#include "die.h"
#include "FileBrowser.h"
#include "FileBrowserSort.h"

void
loop(void)
{
	ALLEGRO_KEYBOARD_STATE kbState;
	bool redraw = false, done = false;

	FILEBROWSER *fb = filebrowser_new(SCREEN_W, SCREEN_H);
	const char *curdir = al_get_current_directory();
	ALLEGRO_BITMAP *backbuffer = al_get_backbuffer(display);
	uint8_t fbmode;

	fbsort_init();
	filebrowser_set_hook(fb, FILEBROWSER_HOOK_DIRSORT, fbsort_dirs);
	filebrowser_set_hook(fb, FILEBROWSER_HOOK_FILESORT, fbsort_file);

	filebrowser_load_font(fb, FILEBROWSER_FONT_DEFAULT, DEFAULT_FONT,
		DEFAULT_FONT_SIZE, DEFAULT_FONT_FLAG);

	if (curdir)
		filebrowser_browse_path(fb, curdir);

	filebrowser_draw(fb);
	al_set_target_bitmap(backbuffer);
	al_clear_to_color(COLOR_NORMAL_BLACK);
	al_draw_bitmap(filebrowser_bitmap(fb), 0, 0, 0);
	al_flip_display();

	for (int i = 0; i < TIMER_MAX; i++) {
		if (timers[i] != NULL)
			al_start_timer(timers[i]);
	}

	while (!done) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		switch (ev.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			done = true;
			redraw = false;
			break;
		case ALLEGRO_EVENT_KEY_UP:
			switch (ev.keyboard.keycode) {
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				redraw = false;
				break;
			case ALLEGRO_KEY_PGUP:
			case ALLEGRO_KEY_PGDN:
				break;
			case ALLEGRO_KEY_UP:
				redraw = filebrowser_select_prev(fb);
				break;
			case ALLEGRO_KEY_DOWN:
				redraw = filebrowser_select_next(fb);
				break;
			case ALLEGRO_KEY_ENTER:
				fbmode = filebrowser_draw_mode(fb, 0);
				if (fbmode & FILEBROWSER_DRAW_FILEINFO) {
					filebrowser_draw_mode(fb, FILEBROWSER_DRAW_DIRLIST);
					filebrowser_draw(fb);
					redraw = true;
				}
				else {
					redraw = filebrowser_browse_selected(fb);
				}
				break;
			case ALLEGRO_KEY_BACKSPACE:
				redraw = filebrowser_browse_parent(fb);
				break;
			case ALLEGRO_KEY_SPACE:
				redraw = true;
				filebrowser_draw_mode(fb, FILEBROWSER_DRAW_DIRLIST);
				filebrowser_draw(fb);
				break;
			default:
				break;
			}
			break;
		case ALLEGRO_EVENT_TIMER:
			if (ev.timer.source == timers[TIMER_MAIN]) {
				al_get_keyboard_state(&kbState);
			}
			break;
		default:
			break;
		}

		if (redraw && al_is_event_queue_empty(event_queue)) {
			redraw = false;
			filebrowser_draw(fb);
			al_set_target_bitmap(backbuffer);
			al_clear_to_color(COLOR_NORMAL_BLACK);
			al_draw_bitmap(filebrowser_bitmap(fb), 0, 0, 0);
			al_flip_display();
		}
	}
	fbsort_cleanup();
	filebrowser_destroy(fb);
}

