#include "helpmenu.h"

#include <stdlib.h>
#include <assert.h>

typedef struct HELP_MENU_FONT {
	ALLEGRO_FONT *font;
	int size;
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

#define HELP_TITLE "Help"

void
helpmenu_draw(HELP_MENU *help)
{
	static ALLEGRO_COLOR bg, fg, color_border;
	static HELP_MENU_FONT F;
	static int w, h;

	assert(help != NULL);

	bg = help->colors[HELP_MENU_COLOR_BACKGROUND];
	fg = help->colors[HELP_MENU_COLOR_FOREGROUND];
	color_border = help->colors[HELP_MENU_COLOR_BORDER];
	F = help->fonts[HELP_MENU_FONT_DEFAULT];

	w = help->w;
	h = help->h;

	int p = 2;
	int lh = F.size / 2 + p;

	al_set_target_bitmap(help->b);
	al_clear_to_color(bg);
	al_draw_rectangle(p, lh, w - p, h, color_border, 1);

	int tw = al_get_text_width(F.font, HELP_TITLE);
	int center = w / 2 - tw / 2;
	al_draw_filled_rectangle(center - 1, p, (w / 2) + (tw / 2) + 1, lh, bg);
	al_draw_text(F.font, fg, center, p, 0, HELP_TITLE);

	al_draw_line(w / 2, F.size + 2*p, w / 2, h - p, color_border, 1);
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
		h->fonts[id].flags = fi.flags;
	}

	return true;
}
