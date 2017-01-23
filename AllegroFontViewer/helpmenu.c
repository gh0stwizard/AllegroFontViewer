#include "helpmenu.h"
#include "colors.h"

#include <stdlib.h>
#include <assert.h>

typedef struct HELP_MENU_FONT {
	ALLEGRO_FONT *font;
	int size;
	int height;
	int flags;
} HELP_MENU_FONT;

//typedef struct HELP_MENU_DATA {
//
//};

//enum {
//	HELP_MENU_DATA_DIRSLIST,
//	HELP_MENU_DATA_FONTVIEW,
//	HELP_MENU_DATA_TYPING,
//	HELP_MENU_DATA_MAX
//};

struct HELP_MENU {
	ALLEGRO_BITMAP *b;
	int w, h;
	ALLEGRO_COLOR colors[HELP_MENU_COLOR_MAX];
	HELP_MENU_FONT fonts[HELP_MENU_FONT_MAX];
	/*struct {
		ALLEGRO_USTR *title;
		HELP_MENU_DATA data[HELP_MENU_DATA_MAX];
	} data;*/
};


HELP_MENU *
helpmenu_new(int width, int height)
{
	HELP_MENU *h = calloc(1, sizeof(HELP_MENU));
	assert(h != NULL);

	h->w = width;
	h->h = height;
	h->b = al_create_bitmap(h->w, h->h);
	assert(h->b != NULL);

	return h;
}

void
helpmenu_destroy(HELP_MENU *h)
{
	if (h == NULL)
		return;

	free(h);
}

ALLEGRO_BITMAP *
helpmenu_bitmap(HELP_MENU *h)
{
	assert(h != NULL);
	return h->b;
}

static ALLEGRO_BITMAP *
create_hotkey_bitmap(ALLEGRO_FONT *font, const char *text, ALLEGRO_COLOR bg, 
	ALLEGRO_COLOR fg)
{
	static int w, h;
	static ALLEGRO_BITMAP *bmp;

	w = al_get_text_width(font, text);
	h = al_get_font_line_height(font);
	bmp = al_create_bitmap(w, h);
	al_set_target_bitmap(bmp);
	al_clear_to_color(bg);
	al_draw_text(font, fg, 0, 0, 0, text);

	return bmp;
}

typedef struct KLDAT {
	int xdelta;
	ALLEGRO_FONT *font;
	ALLEGRO_BITMAP *bb;
	ALLEGRO_COLOR bg, fg;
	ALLEGRO_COLOR keybg, keyfg;
	int x1, y1;
	int x2, y2;
} KLDAT;

static int
draw_keylist(int start, int end, char *desc[], int keys[], KLDAT *data)
{
	static ALLEGRO_BITMAP *keybmp;

	for (int i = start; (i < end) && (desc[i] != NULL); i++) {
		keybmp = create_hotkey_bitmap(data->font, al_keycode_to_name(keys[i]), 
			data->keybg, data->keyfg);
		al_set_target_bitmap(data->bb);
		al_draw_text(data->font, data->fg, data->x1, data->y1, 0, desc[i]);
		al_draw_bitmap(keybmp, data->x2, data->y2, 0);
		al_destroy_bitmap(keybmp);
		data->y1 += data->xdelta;
		data->y2 = data->y1;
	}

	return data->y1;
}

void
helpmenu_draw(HELP_MENU *help)
{
	static ALLEGRO_COLOR bg, fg, color_border;
	static HELP_MENU_FONT F;
	static int w, h;
	static ALLEGRO_BITMAP *b;

	assert(help != NULL);

	bg = help->colors[HELP_MENU_COLOR_BACKGROUND];
	fg = help->colors[HELP_MENU_COLOR_FOREGROUND];
	color_border = help->colors[HELP_MENU_COLOR_BORDER];
	F = help->fonts[HELP_MENU_FONT_DEFAULT];

	w = help->w;
	h = help->h;
	b = help->b;

	int px = 10;
	int py = 2;
	int lh = F.size / 2 + py;

	al_set_target_bitmap(b);
	al_clear_to_color(bg);
	al_draw_rectangle(py, lh, w - py, h, color_border, 1);

#define HELP_TITLE "Help"

	int tw = al_get_text_width(F.font, HELP_TITLE);
	int center = w / 2 - tw / 2;
	al_draw_filled_rectangle(center - 1, py, (w / 2) + (tw / 2) + 1, lh, bg);
	al_draw_text(F.font, fg, center, py, 0, HELP_TITLE);

#undef HELP_TITLE

	char *desc[] = {
		"Exit from the program",
		"Show this help",
		"Reload configuration",
		/* directory listing */
		"Navigate Up",
		"Navigate Down",
		"Navigate Up Quickly",
		"Navigate Down Quickly",
		"Open a parent directory",
		"Enter into a selected directory or load a font",
		"Switch between this mode and Font View Mode",
		/* font view mode */
		"Switch to the Typing Mode",
		"Go back to the Directory Listing Mode",
		"Load a preset",
		/* typing mode */
		"Apply a typed text",
		"Remove a previously typed character",
		"Clear the whole typed text",
		"Load a preset"
	};
	int keys[] = {
		ALLEGRO_KEY_ESCAPE,
		ALLEGRO_KEY_F1,
		ALLEGRO_KEY_F12,
		/* directory listing */
		ALLEGRO_KEY_UP,
		ALLEGRO_KEY_DOWN,
		ALLEGRO_KEY_PGUP,
		ALLEGRO_KEY_PGDN,
		ALLEGRO_KEY_BACKSPACE,
		ALLEGRO_KEY_ENTER,
		ALLEGRO_KEY_SPACE,
		/* font view mode */
		ALLEGRO_KEY_INSERT,
		ALLEGRO_KEY_ENTER,
		ALLEGRO_KEY_F2,
		/* typing mode */
		ALLEGRO_KEY_ENTER,
		ALLEGRO_KEY_BACKSPACE,
		ALLEGRO_KEY_DELETE,
		ALLEGRO_KEY_F2
	};

	lh = F.height + 2 * py;
	ALLEGRO_COLOR color_heading = COLOR_BRIGHT_YELLOW;

	al_draw_text(F.font, color_heading, px, lh, 0, "Controls");
	lh += F.height + py;

	KLDAT dat = {
		.x1 = px,
		.y1 = lh,
		.x2 = 3 * w / 4,
		.y2 = lh,
		.xdelta = F.height + py,
		.font = F.font,
		.bb = b,
		.bg = bg,
		.fg = fg,
		.keybg = al_color_name("orange"),
		.keyfg = al_color_name("black")
	};

	lh = draw_keylist(0, 3, desc, keys, &dat);

	lh += F.height + py;
	al_draw_text(F.font, color_heading, px, lh, 0, 
		"Controls: Directory Listing Mode");
	lh += F.height + py;
	dat.y1 = lh;
	dat.y2 = lh;
	lh = draw_keylist(3, 10, desc, keys, &dat);

	lh += F.height + py;
	al_draw_text(F.font, color_heading, px, lh, 0, "Controls: Font View Mode");
	lh += F.height + py;
	dat.y1 = lh;
	dat.y2 = lh;
	lh = draw_keylist(10, 13, desc, keys, &dat);
	al_draw_text(F.font, fg, px, lh, 0, 
		"Point a Mouse to see a font information");

	lh += 2 * (F.height + py);
	al_draw_text(F.font, color_heading, px, lh, 0, "Controls: Typing Mode");
	lh += F.height + py;
	dat.y1 = lh;
	dat.y2 = lh;
	lh = draw_keylist(13, 17, desc, keys, &dat);
//	al_draw_text(F.font, fg, px, lh, 0, "Point a Mouse to a font information");
}

void
helpmenu_set_colors(HELP_MENU *h, ALLEGRO_COLOR list[])
{
	assert(h != NULL);

	for (int i = 0; i < HELP_MENU_COLOR_MAX; i++) {
		h->colors[i] = list[i];
	}
}

bool
helpmenu_load_font(HELP_MENU *h, int id, FONT_INFO fi)
{
	static ALLEGRO_FONT *font;

	assert(h != NULL);
	assert(id < HELP_MENU_FONT_MAX);

	font = al_load_font(fi.file, fi.size, fi.flags);
	assert(font != NULL);

	if (font != NULL) {
		h->fonts[id].font = font;
		h->fonts[id].size = fi.size;
		h->fonts[id].height = al_get_font_line_height(font);
		h->fonts[id].flags = fi.flags;
	}

	return true;
}
