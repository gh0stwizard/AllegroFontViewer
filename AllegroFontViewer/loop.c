#include "loop.h"
#include "main.h"
#include "colors.h"
#include "messagebox.h"
#include "FileBrowser.h"
#include "FontViewer.h"
#include "typer.h"
#ifndef _NO_ICU
#include "FileBrowserSort.h"
#endif

#include <stdio.h>

static FILEBROWSER		*browser;
static FONTVIEWER		*viewer;
static ALLEGRO_BITMAP	*backbuffer;
static TYPER			*typer;

static void draw(ALLEGRO_BITMAP *bmp);

enum {
	STATE_NONE,
	STATE_DIRSLIST,
	STATE_FONTVIEW,
	STATE_TYPING
};

enum {
	FONTVIEW_KEY_ENTER = 1 << 0,
	FONTVIEW_KEY_INSERT = 1 << 1
};

enum {
	FILEINFO_KEY_ENTER = 1 << 0
};

enum {
	BROWSER_KEY_ENTER = 1 << 0,
	BROWSER_KEY_UP = 1 << 1,
	BROWSER_KEY_DOWN = 1 << 2,
	BROWSER_KEY_PGUP = 1 << 3,
	BROWSER_KEY_PGDN = 1 << 4,
	BROWSER_KEY_BACKSPACE = 1 << 5,
	BROWSER_KEY_SPACE = 1 << 6
};


#define LOG(msg) (statusline_push_cstr(status, EVENT_SYSTEM_LOG, msg))
#define SAY(msg) (statusline_push(status, EVENT_SYSTEM_LOG, msg))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/* XXX */
static ALLEGRO_USTR *status_ustr;


static bool
try_load_font(void)
{
	static ALLEGRO_PATH *p;

	if (!(filebrowser_browse_selected(browser))) {
		p = filebrowser_get_selected_path(browser);
		if (p != NULL) {
			if (fontviewer_load_path(viewer, p)) {
				fontviewer_draw(viewer);
				al_destroy_path(p);
				return true;
			}
			else
				al_destroy_path(p);
		}
	}

	return false;
}

void
loop(void)
{
	ALLEGRO_KEYBOARD_STATE kbState;
	bool redraw = false, done = false;
	int scrlspeed = CFG->browser.scrollspeed;
	int state = STATE_DIRSLIST;
	bool keys[ALLEGRO_KEY_MAX];

	int pressed = 0;
	int dirslist_buttons[] = {
		BROWSER_KEY_UP,
		BROWSER_KEY_DOWN,
		BROWSER_KEY_PGUP,
		BROWSER_KEY_PGDN
	};
	int dirslist_keycodes[] = {
		ALLEGRO_KEY_UP,
		ALLEGRO_KEY_DOWN,
		ALLEGRO_KEY_PGUP,
		ALLEGRO_KEY_PGDN
	};

	int fileinfo_buttons[] = {
		FILEINFO_KEY_ENTER
	};
	int fileinfo_keycodes[] = {
		ALLEGRO_KEY_0
	};

	int fontview_buttons[] = {
		//FONTVIEW_KEY_ENTER,
		FONTVIEW_KEY_INSERT
	};
	int fontview_keycodes[] = {
		//ALLEGRO_KEY_ENTER,
		ALLEGRO_KEY_INSERT
	};

	int w = CFG->display.w;
	int h = CFG->display.h - 20;

	browser = filebrowser_new(w, h);
	viewer = fontviewer_new(w, h);
	backbuffer = al_get_backbuffer(display);
	typer = typer_new(w, h);

	typer_load_font(typer,
		CFG->fonts[FONT_BROWSER].file,
		CFG->fonts[FONT_BROWSER].size,
		CFG->fonts[FONT_BROWSER].flags);

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
			default:
				redraw = false;
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
			case ALLEGRO_KEY_F12:
				config_destroy(CFG);
				CFG = config_new(NULL);
				filebrowser_set_colors(browser, CFG->browser.colors);
				scrlspeed = CFG->browser.scrollspeed;
				break;
			case ALLEGRO_KEY_SPACE:
				switch (state) {
				case STATE_DIRSLIST:
					if (fontviewer_is_drawn(viewer))
						state = STATE_FONTVIEW;
					break;
				case STATE_FONTVIEW:
					state = STATE_DIRSLIST;
					break;
				}
				break;
			case ALLEGRO_KEY_ENTER:
				switch (state) {
				case STATE_DIRSLIST:
					/* when selected is not a directory ... */
					if (try_load_font()) {
						state = STATE_FONTVIEW;
					}
					/* otherwise just draw a new selected directory listing! */
					break;
				case STATE_FONTVIEW:
					state = STATE_DIRSLIST;
					break;
				case STATE_TYPING:
					state = STATE_FONTVIEW;
					fontviewer_set_text(viewer, typer_get_text(typer));
					fontviewer_draw(viewer);
					break;
				}
				break;
			case ALLEGRO_KEY_INSERT:
				switch (state) {
				case STATE_FONTVIEW:
					state = STATE_TYPING;
					if (al_ustr_size(typer_get_text(typer)) == 0)
						LOG("Type a text...");
					else
						SAY(typer_get_text(typer));
					break;
				}
				break;
			case ALLEGRO_KEY_BACKSPACE:
				switch (state) {
				case STATE_DIRSLIST:
					redraw = filebrowser_browse_parent(browser);
					break;
				}
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
				pressed = 0;
				al_get_keyboard_state(&kbState);

				switch (state) {
				case STATE_DIRSLIST:
					for (int i = 0; i < ARRAY_SIZE(dirslist_buttons); i++) {
						if (al_key_down(&kbState, dirslist_keycodes[i]))
							pressed += dirslist_buttons[i];
					}

					switch (pressed) {
					case BROWSER_KEY_UP:
						redraw = filebrowser_select_prev(browser);
						break;
					case BROWSER_KEY_DOWN:
						redraw = filebrowser_select_next(browser);
						break;
					case BROWSER_KEY_PGUP:
						redraw = filebrowser_select_prev_items(browser,
							scrlspeed);
						break;
					case BROWSER_KEY_PGDN:
						redraw = filebrowser_select_next_items(browser,
							scrlspeed);
						break;
					}
					break;

				case STATE_TYPING:
					redraw = true;
					for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
						keys[i] = al_key_down(&kbState, i);
						if (keys[i]) {
							switch (i) {
							case ALLEGRO_KEY_F2:
								typer_set_text(typer, CFG->viewer.presets[0]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F3:
								typer_set_text(typer, CFG->viewer.presets[1]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F4:
								typer_set_text(typer, CFG->viewer.presets[2]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F5:
								typer_set_text(typer, CFG->viewer.presets[3]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F6:
								typer_set_text(typer, CFG->viewer.presets[4]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F7:
								typer_set_text(typer, CFG->viewer.presets[5]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F8:
								typer_set_text(typer, CFG->viewer.presets[6]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F9:
								typer_set_text(typer, CFG->viewer.presets[7]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F10:
								typer_set_text(typer, CFG->viewer.presets[8]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F11:
								typer_set_text(typer, CFG->viewer.presets[9]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								fontviewer_draw(viewer);
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F1:
							case ALLEGRO_KEY_INSERT:
							case ALLEGRO_KEY_ESCAPE:
							case ALLEGRO_KEY_ENTER:
								break;
							case ALLEGRO_KEY_DELETE:
								typer_truncate(typer);
								SAY(typer_get_text(typer));
								break;
							case ALLEGRO_KEY_BACKSPACE:
								typer_remove(typer);
								SAY(typer_get_text(typer));
								break;
							case ALLEGRO_KEY_SPACE:
								typer_type(typer, " ");
								SAY(typer_get_text(typer));
								break;
							default:
								typer_type(typer, al_keycode_to_name(i));
								SAY(typer_get_text(typer));
								break;
							}
						}
					}
					break;
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
				draw(filebrowser_bitmap(browser));
				break;
			case STATE_FONTVIEW:
				draw(fontviewer_bitmap(viewer));
				break;
			case STATE_TYPING:
				draw(fontviewer_bitmap(viewer));
				break;
			default:
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
	if (bmp != NULL)
		al_draw_bitmap(bmp, 0, 0, 0);
	draw_status();
	al_flip_display();
}
