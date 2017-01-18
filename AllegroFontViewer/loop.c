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

#include <stdio.h>

static FILEBROWSER		*browser;
static FONTVIEWER		*viewer;
static ALLEGRO_BITMAP	*backbuffer;

static void draw(ALLEGRO_BITMAP *bmp);

enum {
	STATE_DIRSLIST,
	STATE_FILEINFO,
	STATE_FONTVIEW,
	STATE_BROWSER_ERROR,
	STATE_FONTVIEW_ERROR
};


#define LOG_ERROR(msg) (statusline_push(status, EVENT_SYSTEM_LOG_ERROR, msg))

/* XXX */
static ALLEGRO_USTR *status_ustr;

void
loop(void)
{
	ALLEGRO_KEYBOARD_STATE kbState;
	ALLEGRO_PATH *alpath;
	bool redraw = false, done = false;
	int fast_browse_percent = CFG->browser.scrollspeed;
	int state = STATE_DIRSLIST;

	int w = CFG->display.w;
	int h = CFG->display.h - 20;

	browser = filebrowser_new(w, h);
	viewer = fontviewer_new(w, h);
	backbuffer = al_get_backbuffer(display);

	status = statusline_new(EVENT_TYPE_STATUSLINE);
	al_register_event_source(event_queue, (ALLEGRO_EVENT_SOURCE *)status);
	status_ustr = al_ustr_new("");

#ifndef _NO_ICU
	fbsort_init();
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_SORT_DIRS, fbsort_dirs);
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_SORT_FILE, fbsort_file);
#endif

	filebrowser_load_font(browser, FILEBROWSER_FONT_DEFAULT,
		CFG->fonts[FONT_BROWSER].file,
		CFG->fonts[FONT_BROWSER].size,
		CFG->fonts[FONT_BROWSER].flags);

	filebrowser_set_colors(browser, CFG->browser.colors);

	/* show current directory listing immediatly */
	redraw = filebrowser_browse_path(browser, CFG->browser.startpath);

	for (int i = 0; i < TIMER_MAX; i++) {
		if (timers[i] != NULL)
			al_start_timer(timers[i]);
	}

	while (!done) {
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		switch (ev.type) {
		case EVENT_TYPE_STATUSLINE:
			redraw = true;
			switch (ev.user.data1) {
			case EVENT_SYSTEM_LOG:
			case EVENT_SYSTEM_LOG_FATAL:
				al_ustr_truncate(status_ustr, 0);
				al_ustr_append(status_ustr, (void *)ev.user.data2);
				break;
			}
			al_unref_user_event(&ev.user);
			break;

		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			done = true;
			redraw = false;
			break;

		case ALLEGRO_EVENT_KEY_UP:
			redraw = true;
			switch (ev.keyboard.keycode) {
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				redraw = false;
				break;
			case ALLEGRO_KEY_R:
				config_destroy(CFG);
				CFG = config_new(NULL);
				filebrowser_set_colors(browser, CFG->browser.colors);
				fast_browse_percent = CFG->browser.scrollspeed;
				break;
			case ALLEGRO_KEY_F:
				if (fontviewer_is_drawn(viewer))
					state = STATE_FONTVIEW;
				break;
			case ALLEGRO_KEY_SPACE:
				/* TODO: add a settings switch to handle such behavior? */
				if (state == STATE_FONTVIEW) {
					state = STATE_DIRSLIST;
				}
				else {
					alpath = filebrowser_get_selected_path(browser);
					if (alpath != NULL) {
						if (fontviewer_load_path(viewer, alpath)) {
							fontviewer_draw(viewer);
							state = STATE_FONTVIEW;
						}
						else {
							state = STATE_FONTVIEW_ERROR;
						}
						al_destroy_path(alpath);
					}
					else
						state = STATE_BROWSER_ERROR;
				}
				break;
			case ALLEGRO_KEY_PGUP:
				if (filebrowser_select_prev_items(browser,
					fast_browse_percent))
					state = STATE_DIRSLIST;
				else
					state = STATE_BROWSER_ERROR;
				break;
			case ALLEGRO_KEY_PGDN:
				if (filebrowser_select_next_items(browser,
					fast_browse_percent))
					state = STATE_DIRSLIST;
				else
					state = STATE_BROWSER_ERROR;
				break;
			case ALLEGRO_KEY_UP:
				if (filebrowser_select_prev(browser))
					state = STATE_DIRSLIST;
				else
					state = STATE_BROWSER_ERROR;
				break;
			case ALLEGRO_KEY_DOWN:
				if (filebrowser_select_next(browser))
					state = STATE_DIRSLIST;
				else
					state = STATE_BROWSER_ERROR;
				break;
			case ALLEGRO_KEY_ENTER:
				switch (state) {
				case STATE_FONTVIEW:
				case STATE_FILEINFO:
					state = STATE_DIRSLIST;
					break;
				default:
					if (filebrowser_browse_selected(browser)) {
						switch (filebrowser_draw_mode(browser, 0)) {
						case FILEBROWSER_DRAW_DIRS:
							state = STATE_DIRSLIST;
							break;
						case FILEBROWSER_DRAW_INFO:
							state = STATE_FILEINFO;
							break;
						}
					}
					else {
						state = STATE_BROWSER_ERROR;
					}
					break;
				}
				break;
			case ALLEGRO_KEY_BACKSPACE:
				if (filebrowser_browse_parent(browser))
					state = STATE_DIRSLIST;
				else
					state = STATE_BROWSER_ERROR;
				break;

			default:
				redraw = false;
				break;
			}
			break;
		case ALLEGRO_EVENT_TIMER:
			if (ev.timer.source == timers[TIMER_MAIN]) {
				/* FIXME: is this really need ? */
			}
			else if (ev.timer.source == timers[TIMER_KEYBOARD]) {
				al_get_keyboard_state(&kbState);
				for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
					if (al_key_down(&kbState, i)) {
						statusline_push(status, EVENT_SYSTEM_LOG, 
							al_keycode_to_name(i));
					}
				}
			}
			break;
		default:
			break;
		}

		if (redraw && al_is_event_queue_empty(event_queue)) {
			redraw = false;
			switch (state) {
			case STATE_DIRSLIST:
				filebrowser_draw_mode(browser, FILEBROWSER_DRAW_DIRS);
				draw(filebrowser_bitmap(browser));
				break;

			case STATE_FILEINFO:
				filebrowser_draw_mode(browser, FILEBROWSER_DRAW_INFO);
				draw(filebrowser_bitmap(browser));
				break;

			case STATE_FONTVIEW:
				draw(fontviewer_bitmap(viewer));
				break;
			}
		}
	}
#ifndef _NO_ICU
	fbsort_cleanup();
#endif
	filebrowser_destroy(browser);
	fontviewer_destroy(viewer);

	al_ustr_free(status_ustr);
}

static void
draw_status(void)
{
	int h = CFG->display.h;
	int w = CFG->display.w;
	float top = CFG->display.h - 20;
	ALLEGRO_COLOR bg = CFG->status.colors[STATUS_COLOR_BACKGROUND];
	ALLEGRO_COLOR fg = CFG->status.colors[STATUS_COLOR_FOREGROUND];
	/* TODO: get colors from all states/modes */
	ALLEGRO_COLOR bc = CFG->browser.colors[FILEBROWSER_COLOR_BORDER]; 
	ALLEGRO_FONT *font = fonts[FONT_STATUS];
	int ftop = top + (CFG->fonts[FONT_STATUS].size / 2);

	al_draw_filled_rectangle(0, top, w, h, bg);
	al_draw_rectangle(2, top, w - 2, h - 1, bc, 1);
	al_draw_ustr(font, fg, 5, ftop, 0, status_ustr);
}

static void
draw(ALLEGRO_BITMAP *bmp)
{
	filebrowser_draw(browser);
	al_set_target_bitmap(backbuffer);
	al_clear_to_color(COLOR_NORMAL_BLACK);
	al_draw_bitmap(bmp, 0, 0, 0);
	draw_status();
	al_flip_display();
}
