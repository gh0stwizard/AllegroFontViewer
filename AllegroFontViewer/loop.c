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

static FILEBROWSER *fb;
static FONTVIEWER *fview;
static ALLEGRO_BITMAP *backbuffer;

static void
draw(const ALLEGRO_BITMAP *bmp)
{
	filebrowser_draw(fb);
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
	if (filebrowser_browse_path(fb, curdir))
		draw(filebrowser_bitmap(fb));
}

void
loop(void)
{
	ALLEGRO_KEYBOARD_STATE kbState;
	ALLEGRO_PATH *alpath;
	uint8_t fbmode;
	const char *pathstr;
	bool result;
	bool redraw = false, done = false;

	fb = filebrowser_new(SCREEN_W, SCREEN_H);
	fview = fontviewer_new(SCREEN_W, SCREEN_H);
	backbuffer = al_get_backbuffer(display);

#ifndef _NO_ICU
	fbsort_init();
	filebrowser_set_hook(fb, FILEBROWSER_HOOK_DIRSORT, fbsort_dirs);
	filebrowser_set_hook(fb, FILEBROWSER_HOOK_FILESORT, fbsort_file);
#endif

	filebrowser_load_font(fb, FILEBROWSER_FONT_DEFAULT, DEFAULT_FONT,
		DEFAULT_FONT_SIZE, DEFAULT_FONT_FLAG);

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
				alpath = filebrowser_get_selected_path(fb);
				pathstr = al_path_cstr(alpath, ALLEGRO_NATIVE_PATH_SEP);
				result = fontviewer_load(fview, pathstr);
				if (result) {
					fontviewer_draw(fview);
					draw(fontviewer_bitmap(fview));
				}
				else {
					mbox_note("font error", pathstr);
				}
				al_destroy_path(alpath);
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
			draw(filebrowser_bitmap(fb));
		}
	}
#ifndef _NO_ICU
	fbsort_cleanup();
#endif
	filebrowser_destroy(fb);
	fontviewer_destroy(fview);
}

