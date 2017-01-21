#include "helpmenu.h"

#include <stdlib.h>
#include <assert.h>

typedef struct HELP_MENU_FONT {
	ALLEGRO_FONT *font;
	int size;
	int flags;
} HELP_MENU_FONT;

struct HELP_MENU {
	ALLEGRO_BITMAP *b;
	int w, h;
	ALLEGRO_COLOR colors[HELP_MENU_COLOR_MAX];
	HELP_MENU_FONT fonts[HELP_MENU_FONT_MAX];
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

void
helpmenu_draw(HELP_MENU *h)
{
	ALLEGRO_COLOR bg, fg;

	assert(h != NULL);

	bg = h->colors[HELP_MENU_COLOR_BACKGROUND];
	fg = h->colors[HELP_MENU_COLOR_FOREGROUND];

	al_set_target_bitmap(h->b);
	al_clear_to_color(bg);

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
