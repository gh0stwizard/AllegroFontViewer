#include "main.h"
#include "colors.h"
#include "FileBrowser.h"
#include "FontViewer.h"
#include "typer.h"
#include "helpmenu.h"
#include "evlog.h"
#include "error.h"
#ifndef _NO_ICU
#include "FileBrowserSort.h"
#endif

#include <stdlib.h>
#include <assert.h>

static FILEBROWSER		*browser;
static FONTVIEWER		*viewer;
static ALLEGRO_BITMAP	*backbuffer;
static TYPER			*typer;
static HELP_MENU		*help;
static EVLOG			*statuslog;
static ERROR			*errwin;
static STATUSLINE		*statusbar;

#define SAYF(fmt, ...) (evlog_pushf(statuslog, EVENT_SYSTEM_STATUS, fmt, __VA_ARGS__))
#define SAY(us) (evlog_push(statuslog, EVENT_SYSTEM_STATUS, us))

static void
draw(ALLEGRO_BITMAP *bmp);

static bool
try_load_font(void);

enum {
	STATE_DIRSLIST = 1,
	STATE_FONTVIEW,
	STATE_TYPING,
	STATE_ERROR,
	STATE_HELP
};

enum {
	BROWSER_KEY_ENTER = 1 << 0,
	BROWSER_KEY_UP = 1 << 1,
	BROWSER_KEY_DOWN = 1 << 2,
	BROWSER_KEY_PGUP = 1 << 3,
	BROWSER_KEY_PGDN = 1 << 4,
	BROWSER_KEY_BACKSPACE = 1 << 5,
	BROWSER_KEY_SPACE = 1 << 6,

	BROWSER_KEY_LSHIFT = 0xa0,
	BROWSER_KEY_LSHIFT_K = 0xa2,
	BROWSER_KEY_LSHIFT_J = 0xa4,
};

void
loop(void)
{
	ALLEGRO_KEYBOARD_STATE kbState;
	ALLEGRO_MOUSE_STATE mState;
	ERROR_DATA edata;
	int fontsize;
	FONT_ATTR *fontattr;
	bool redraw = false, done = false;
	int scrlspeed = CFG->browser.scrollspeed;
	int state = STATE_DIRSLIST;
	bool keys[ALLEGRO_KEY_MAX];

	int pressed = 0;
	int dirslist_buttons[] = {
		BROWSER_KEY_UP,
		BROWSER_KEY_UP, /*vim-like*/
		BROWSER_KEY_DOWN,
		BROWSER_KEY_DOWN, /*vim-like*/
		BROWSER_KEY_PGUP,
		BROWSER_KEY_PGDN,
		BROWSER_KEY_LSHIFT
	};
	int dirslist_keycodes[] = {
		ALLEGRO_KEY_UP,
		ALLEGRO_KEY_K, /*vim-like*/
		ALLEGRO_KEY_DOWN,
		ALLEGRO_KEY_J, /*vim-like*/
		ALLEGRO_KEY_PGUP,
		ALLEGRO_KEY_PGDN,
		ALLEGRO_KEY_LSHIFT
	};

	const int statusbar_h = 20;
	const int border_px = 2;
	const int border_py = 2;
	int w = CFG->display.w;
	int h = CFG->display.h - statusbar_h;

	backbuffer = al_get_backbuffer(display);

	browser = filebrowser_new(w, h);
	filebrowser_set_colors(browser, CFG->browser.colors);
	filebrowser_load_fonts(browser, CFG->browser.fonts);

#ifndef _NO_ICU
	fbsort_init();
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_SORT_DIRS, fbsort_dirs);
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_SORT_FILE, fbsort_file);
#endif

	viewer = fontviewer_new(w, h);
	fontviewer_set_colors(viewer, CFG->viewer.colors);
	fontviewer_set_font_size_limits(viewer,
		CFG->viewer.minsize, CFG->viewer.maxsize);

	typer = typer_new(w, h);

	help = helpmenu_new(w, h);
	helpmenu_load_fonts(help, CFG->help.fonts);
	helpmenu_set_colors(help, CFG->help.colors);
	helpmenu_draw(help);

	statuslog = evlog_new(EVENT_TYPE_STATUS);
	statusbar = statusline_new(w, statusbar_h, border_px, border_py);
	statusline_load_fonts(statusbar, CFG->status.fonts);
	statusline_set_colors(statusbar, CFG->status.colors);
	statusline_draw(statusbar, NULL);

	errwin = error_new(w, h);
	error_load_fonts(errwin, CFG->error.fonts);
	error_set_colors(errwin, CFG->error.colors);

	al_register_event_source(event_queue, (ALLEGRO_EVENT_SOURCE *)statuslog);
	al_register_event_source(event_queue, (ALLEGRO_EVENT_SOURCE *)viewer);

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
		case EVENT_TYPE_ERROR:
			redraw = true;
			edata.file = (void *)ev.user.data1;
			edata.function = (void *)ev.user.data2;
			edata.line = (void *)ev.user.data3;
			edata.message = (void *)ev.user.data4;
			error_draw(errwin, &edata);
			state = STATE_ERROR;
			al_unref_user_event(&ev.user);
			break;

		case EVENT_TYPE_STATUS:
			redraw = true;
			statusline_draw(statusbar, (void *)ev.user.data2);
			al_unref_user_event(&ev.user);
			break;

		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			done = true;
			redraw = false;
			break;

		case ALLEGRO_EVENT_MOUSE_AXES:
			switch (state) {
			case STATE_FONTVIEW:
				al_get_mouse_state(&mState);
				fontsize = fontviewer_get_font_size_mouse(viewer, 
					mState.x, mState.y);
				fontattr = fontviewer_get_attr_by_size(viewer, fontsize);
				if (fontattr != NULL) {
					SAYF("Size: %d  Height: %d  Ascent: %d  Descent: %d",
						fontsize, fontattr->height,
						fontattr->ascent, fontattr->descent);
					free(fontattr);
				}
				break;
			default:
				break;
			}
			break;

		case ALLEGRO_EVENT_KEY_UP:
			redraw = true;
			switch (ev.keyboard.keycode) {
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				redraw = false;
				break;
			case ALLEGRO_KEY_F1:
				switch (state) {
				case STATE_HELP:
					state = STATE_DIRSLIST; /* FIXME */
					break;
				default:
					state = STATE_HELP;
					break;
				}
				break;
			case ALLEGRO_KEY_F12:
				config_destroy(CFG);
				CFG = config_new(NULL);
				scrlspeed = CFG->browser.scrollspeed;
				filebrowser_set_colors(browser, CFG->browser.colors);
				fontviewer_set_colors(viewer, CFG->viewer.colors);
				error_set_colors(errwin, CFG->error.colors);
				helpmenu_set_colors(help, CFG->help.colors);
				statusline_set_colors(statusbar, CFG->status.colors);
				helpmenu_draw(help);
				statusline_draw(statusbar, NULL);
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
			case ALLEGRO_KEY_F: /* vim-like */
				switch (state) {
				case STATE_DIRSLIST:
					/* when selected is not a directory ... */
					if (try_load_font())
						state = STATE_FONTVIEW;
					/* otherwise just draw a new selected directory listing! */
					break;
				case STATE_ERROR:
				case STATE_FONTVIEW:
					state = STATE_DIRSLIST;
					break;
				}
				break;
			case ALLEGRO_KEY_ENTER:
				switch (state) {
				case STATE_DIRSLIST:
					/* when selected is not a directory ... */
					if (try_load_font())
						state = STATE_FONTVIEW;
					/* otherwise just draw a new selected directory listing! */
					break;
				case STATE_ERROR:
				case STATE_FONTVIEW:
					state = STATE_DIRSLIST;
					break;
				case STATE_TYPING:
					state = STATE_FONTVIEW;
					fontviewer_set_text(viewer, typer_get_text(typer));
					statusline_noblink(statusbar);
					statusline_draw(statusbar, typer_get_text(typer));
					break;
				default:
					break;
				}
				break;
			case ALLEGRO_KEY_I:
			case ALLEGRO_KEY_INSERT:
				switch (state) {
				case STATE_FONTVIEW:
					state = STATE_TYPING;
					if (al_ustr_size(typer_get_text(typer)) == 0)
						SAYF("Type a text...");
					else
						SAY(typer_get_text(typer));
					break;
				}
				break;
			case ALLEGRO_KEY_D:
				switch (state) {
				case STATE_DIRSLIST:
					filebrowser_browse_parent(browser);
					break;
				case STATE_FONTVIEW:
					state = STATE_DIRSLIST;
					break;
				}
				break;
			case ALLEGRO_KEY_BACKSPACE:
				switch (state) {
				case STATE_DIRSLIST:
					filebrowser_browse_parent(browser);
					break;
				case STATE_FONTVIEW:
					state = STATE_DIRSLIST;
					break;
				}
				break;
			case ALLEGRO_KEY_F2:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[0]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F3:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[1]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F4:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[2]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F5:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[3]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F6:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[4]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F7:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[5]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F8:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[6]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F9:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[7]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F10:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[8]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F11:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[9]);
					fontviewer_set_text(viewer, typer_get_text(typer));
					SAY(typer_get_text(typer));
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
			else if (ev.timer.source == timers[TIMER_BLINK]) {
				if (state == STATE_TYPING) {
					redraw = true;
					statusline_blink(statusbar);
					statusline_draw(statusbar, typer_get_text(typer));
				}
				else
					statusline_noblink(statusbar);
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
					case BROWSER_KEY_LSHIFT_K:
					case BROWSER_KEY_PGUP:
						redraw = filebrowser_select_prev_items(browser,
							scrlspeed);
						break;
					case BROWSER_KEY_LSHIFT_J:
					case BROWSER_KEY_PGDN:
						redraw = filebrowser_select_next_items(browser,
							scrlspeed);
						break;
					}
					break;

				case STATE_TYPING:
					for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
						keys[i] = al_key_down(&kbState, i);
						if (keys[i]) {
							redraw = true;
							switch (i) {
							case ALLEGRO_KEY_F2:
								typer_set_text(typer, CFG->viewer.presets[0]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F3:
								typer_set_text(typer, CFG->viewer.presets[1]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F4:
								typer_set_text(typer, CFG->viewer.presets[2]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F5:
								typer_set_text(typer, CFG->viewer.presets[3]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F6:
								typer_set_text(typer, CFG->viewer.presets[4]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F7:
								typer_set_text(typer, CFG->viewer.presets[5]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F8:
								typer_set_text(typer, CFG->viewer.presets[6]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F9:
								typer_set_text(typer, CFG->viewer.presets[7]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F10:
								typer_set_text(typer, CFG->viewer.presets[8]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F11:
								typer_set_text(typer, CFG->viewer.presets[9]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F1:
							case ALLEGRO_KEY_INSERT:
							case ALLEGRO_KEY_ESCAPE:
							case ALLEGRO_KEY_ALT:
							case ALLEGRO_KEY_ALTGR:
							case ALLEGRO_KEY_HOME:
							case ALLEGRO_KEY_END:
							case ALLEGRO_KEY_LCTRL:
							case ALLEGRO_KEY_LSHIFT:
							case ALLEGRO_KEY_LWIN:
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
				statusline_draw(statusbar, NULL);
				filebrowser_draw(browser);
				draw(filebrowser_bitmap(browser));
				break;
			case STATE_FONTVIEW:
				fontviewer_draw(viewer);
				draw(fontviewer_bitmap(viewer));
				break;
			case STATE_TYPING:
				draw(fontviewer_bitmap(viewer));
				break;
			case STATE_HELP:
				statusline_draw(statusbar, NULL);
				draw(helpmenu_bitmap(help));
				break;
			case STATE_ERROR:
				statusline_draw(statusbar, NULL);
				draw(error_bitmap(errwin));
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
	helpmenu_destroy(help);
	typer_destroy(typer);
	error_destroy(errwin);
	evlog_destroy(statuslog);
	statusline_destroy(statusbar);
}


static void
draw(ALLEGRO_BITMAP *bmp)
{
	al_set_target_bitmap(backbuffer);
	al_clear_to_color(COLOR_NORMAL_BLACK);
	if (bmp != NULL)
		al_draw_bitmap(bmp, 0, 0, 0);
	al_draw_bitmap(statusline_bitmap(statusbar), 0, CFG->display.h - 20, 0);
	al_flip_display();
}


static bool
try_load_font(void)
{
	static ALLEGRO_PATH *p;

	/* when selected is not a directory ... */
	if (!(filebrowser_browse_selected(browser))) {
		p = filebrowser_get_selected_path(browser);
		if (p != NULL) {
			if (fontviewer_load_path(viewer, p)) {
				al_destroy_path(p);
				return true;
			}
			else
				al_destroy_path(p);
		}
	}

	return false;
}
