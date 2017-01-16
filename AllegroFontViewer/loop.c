#include "loop.h"
#include "main.h"
#include "colors.h"
#include "die.h"
#include "messagebox.h"
#include "FileBrowser.h"
#include "FontViewer.h"
#ifndef _NO_ICU
#include "FileBrowserSort.h"
#endif

static FILEBROWSER		*browser;
static FONTVIEWER		*viewer;
static ALLEGRO_BITMAP	*backbuffer;

static void draw(ALLEGRO_BITMAP *bmp);
static void draw_curdir(void);

void
loop(void)
{
	ALLEGRO_KEYBOARD_STATE kbState;
	ALLEGRO_PATH *alpath;
	uint8_t fbmode;
	const char *pathstr;
	bool result;
	bool redraw = false, done = false;
	const int fast_browse_percent = 50;

	browser = filebrowser_new(SCREEN_W, SCREEN_H);
	viewer = fontviewer_new(SCREEN_W, SCREEN_H);
	backbuffer = al_get_backbuffer(display);

#ifndef _NO_ICU
	fbsort_init();
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_DIRSORT, fbsort_dirs);
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_FILESORT, fbsort_file);
#endif

	filebrowser_load_font(browser, FILEBROWSER_FONT_DEFAULT, DEFAULT_FONT,
		DEFAULT_FONT_SIZE, DEFAULT_FONT_FLAG);

	/* show current directory listing immediatly */
	draw_curdir();

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
			case ALLEGRO_KEY_L:
				alpath = filebrowser_get_selected_path(browser);
				if (alpath != NULL) {
					pathstr = al_path_cstr(alpath, ALLEGRO_NATIVE_PATH_SEP);
					result = fontviewer_load(viewer, pathstr);
					if (result) {
						fontviewer_draw(viewer);
						draw(fontviewer_bitmap(viewer));
					}
					else {
						mbox_note("font error", pathstr);
					}
					al_destroy_path(alpath);
				}
				break;
			case ALLEGRO_KEY_PGUP:
				redraw = filebrowser_select_prev_items(browser, 
					fast_browse_percent);
				break;
			case ALLEGRO_KEY_PGDN:
				redraw = filebrowser_select_next_items(browser,
					fast_browse_percent);
				break;
			case ALLEGRO_KEY_UP:
				redraw = filebrowser_select_prev(browser);
				break;
			case ALLEGRO_KEY_DOWN:
				redraw = filebrowser_select_next(browser);
				break;
			case ALLEGRO_KEY_ENTER:
				fbmode = filebrowser_draw_mode(browser, 0);
				if (fbmode & FILEBROWSER_DRAW_FILEINFO) {
					filebrowser_draw_mode(browser, FILEBROWSER_DRAW_DIRLIST);
					filebrowser_draw(browser);
					redraw = true;
				}
				else {
					redraw = filebrowser_browse_selected(browser);
				}
				break;
			case ALLEGRO_KEY_BACKSPACE:
				redraw = filebrowser_browse_parent(browser);
				break;
			case ALLEGRO_KEY_SPACE:
				redraw = true;
				filebrowser_draw_mode(browser, FILEBROWSER_DRAW_DIRLIST);
				filebrowser_draw(browser);
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
			draw(filebrowser_bitmap(browser));
		}
	}
#ifndef _NO_ICU
	fbsort_cleanup();
#endif
	filebrowser_destroy(browser);
	fontviewer_destroy(viewer);
}

static void
draw(ALLEGRO_BITMAP *bmp)
{
	filebrowser_draw(browser);
	al_set_target_bitmap(backbuffer);
	al_clear_to_color(COLOR_NORMAL_BLACK);
	al_draw_bitmap(bmp, 0, 0, 0);
	al_flip_display();
}

static void
draw_curdir(void)
{
	const char *curdir = al_get_current_directory();
	assert(curdir != NULL);
	if (filebrowser_browse_path(browser, curdir))
		draw(filebrowser_bitmap(browser));
}
