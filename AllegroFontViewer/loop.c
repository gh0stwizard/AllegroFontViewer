#include "loop.h"
#include "main.h"
#include "colors.h"
#include "FileBrowser.h"
#include "FontViewer.h"
#include "typer.h"
#ifndef _NO_ICU
#include "FileBrowserSort.h"
#endif

static FILEBROWSER		*browser;
static FONTVIEWER		*viewer;
static ALLEGRO_BITMAP	*backbuffer;
static TYPER			*typer;

/* XXX */
static ALLEGRO_USTR		*status_ustr;

#define LOG(msg) (statusline_push_cstr(status, EVENT_SYSTEM_LOG, msg))
#define SAY(msg) (statusline_push(status, EVENT_SYSTEM_LOG, msg))

static void draw(ALLEGRO_BITMAP *bmp);
static void draw_error_window(const ALLEGRO_USER_EVENT *ue);
static bool try_load_font(void);

enum {
	STATE_NONE,
	STATE_DIRSLIST,
	STATE_FONTVIEW,
	STATE_TYPING,
	STATE_ERROR
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
	ALLEGRO_USTR *u;
	int fontsize;
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
	al_register_event_source(event_queue, (ALLEGRO_EVENT_SOURCE *)viewer);
	status_ustr = al_ustr_new("");

#ifndef _NO_ICU
	fbsort_init();
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_SORT_DIRS, fbsort_dirs);
	filebrowser_set_hook(browser, FILEBROWSER_HOOK_SORT_FILE, fbsort_file);
#endif

	FONT_INFO fi = {
		.file = CFG->fonts[FONT_BROWSER].file,
		.size = CFG->fonts[FONT_BROWSER].size,
		.flags = CFG->fonts[FONT_BROWSER].flags
	};
	filebrowser_load_font(browser, FILEBROWSER_FONT_DEFAULT, fi);

	filebrowser_set_colors(browser, CFG->browser.colors);

	fontviewer_set_colors(viewer, CFG->viewer.colors);
	fontviewer_set_font_size_limits(viewer,
		CFG->viewer.minsize, CFG->viewer.maxsize);

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
			draw_error_window(&ev.user);
			state = STATE_ERROR;
			break;

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

		case ALLEGRO_EVENT_MOUSE_AXES:
			switch (state) {
			case STATE_FONTVIEW:
				al_get_mouse_state(&mState);
				fontsize = fontviewer_get_font_size_mouse(viewer, mState.x, mState.y);
				u = al_ustr_newf("Font size: %d pixels", fontsize);
				SAY(u);
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
			case ALLEGRO_KEY_F12:
				config_destroy(CFG);
				CFG = config_new(NULL);
				filebrowser_set_colors(browser, CFG->browser.colors);
				fontviewer_set_colors(viewer, CFG->viewer.colors);
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
					LOG("");
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
						LOG("Type a text...");
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
				}
				break;
			case ALLEGRO_KEY_F3:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[1]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F4:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[2]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F5:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[3]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F6:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[4]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F7:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[5]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F8:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[6]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F9:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[7]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F10:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[8]);
					fontviewer_set_text(viewer, typer_get_text(typer));
				}
				break;
			case ALLEGRO_KEY_F11:
				if (state == STATE_FONTVIEW) {
					typer_set_text(typer, CFG->viewer.presets[9]);
					fontviewer_set_text(viewer, typer_get_text(typer));
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
					redraw = true;
					for (int i = 0; i < ALLEGRO_KEY_MAX; i++) {
						keys[i] = al_key_down(&kbState, i);
						if (keys[i]) {
							switch (i) {
							case ALLEGRO_KEY_F2:
								typer_set_text(typer, CFG->viewer.presets[0]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F3:
								typer_set_text(typer, CFG->viewer.presets[1]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F4:
								typer_set_text(typer, CFG->viewer.presets[2]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F5:
								typer_set_text(typer, CFG->viewer.presets[3]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F6:
								typer_set_text(typer, CFG->viewer.presets[4]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F7:
								typer_set_text(typer, CFG->viewer.presets[5]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F8:
								typer_set_text(typer, CFG->viewer.presets[6]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F9:
								typer_set_text(typer, CFG->viewer.presets[7]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F10:
								typer_set_text(typer, CFG->viewer.presets[8]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F11:
								typer_set_text(typer, CFG->viewer.presets[9]);
								fontviewer_set_text(viewer, typer_get_text(typer));
								LOG("");
								state = STATE_FONTVIEW;
								break;
							case ALLEGRO_KEY_F1:
							case ALLEGRO_KEY_INSERT:
							case ALLEGRO_KEY_ESCAPE:
							case ALLEGRO_KEY_ENTER:
							case ALLEGRO_KEY_ALT:
							case ALLEGRO_KEY_ALTGR:
							case ALLEGRO_KEY_HOME:
							case ALLEGRO_KEY_END:
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
	al_set_target_bitmap(backbuffer);
	al_clear_to_color(COLOR_NORMAL_BLACK);
	if (bmp != NULL)
		al_draw_bitmap(bmp, 0, 0, 0);
	draw_status();
	al_flip_display();
}

static void
draw_error_window(const ALLEGRO_USER_EVENT *ue)
{
	ALLEGRO_FONT *font = fonts[FONT_BROWSER];
	static ALLEGRO_USTR *msg;
	const int x = 5;
	const int y = 20;
	int center = CFG->display.w / 2 - al_get_text_width(font, "ERROR") / 2;
	msg = (void *)ue->data4;

	al_set_target_bitmap(backbuffer);
	al_clear_to_color(COLOR_NORMAL_BLACK);
	al_draw_text(font, COLOR_BRIGHT_RED, center, y, 0, "ERROR");
	al_draw_ustr(fonts[FONT_BROWSER], COLOR_BRIGHT_GREEN, x, y + 20, 0, msg);
#if defined(_DEBUG)
	ALLEGRO_USTR *u = al_ustr_new("In function ");
	al_ustr_append(u, (void *)ue->data2);
	al_ustr_append_cstr(u, " at line ");
	al_ustr_append(u, (void *)ue->data3);
	al_ustr_append_cstr(u, ".");
	al_draw_ustr(fonts[FONT_BROWSER], COLOR_BRIGHT_BLUE, x, y + 40, 0, u);
	al_draw_ustr(fonts[FONT_BROWSER], COLOR_BRIGHT_WHITE,
		x, y + 60, 0, (void *)ue->data1);
	al_ustr_free(u);
#endif
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
