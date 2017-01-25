#include "statusline.h"

#include <assert.h>
#include <stdarg.h>


struct STATUSLINE {
	ALLEGRO_BITMAP *b;
	int w, h;
	int px, py;
	ALLEGRO_COLOR colors[STATUS_COLOR_MAX];
	FONT fonts[STATUS_FONT_MAX];
	bool blink;
	int blink_w;
	ALLEGRO_USTR *us;
};


STATUSLINE *
statusline_new(int w, int h, int px, int py)
{
	STATUSLINE *sl = al_malloc(sizeof(STATUSLINE));

	assert(sl != NULL);
	sl->w = w;
	sl->h = h;
	sl->px = px;
	sl->py = py;
	sl->b = al_create_bitmap(sl->w, sl->h);
	assert(sl->b != NULL);

	sl->blink_w = 10;
	sl->blink = false;

	sl->us = al_ustr_new("");

	return sl;
}

void
statusline_destroy(STATUSLINE *sl)
{
	if (sl == NULL)
		return;

	if (sl->b != NULL)
		al_destroy_bitmap(sl->b);

	if (sl->us != NULL)
		al_ustr_free(sl->us);

	for (int i = 0; i < STATUS_FONT_MAX; i++) {
		if (sl->fonts[i].font != NULL) {
			al_destroy_font(sl->fonts[i].font);
			sl->fonts[i].font = NULL;
		}
	}

	al_free(sl);
}


void
statusline_set_colors(STATUSLINE *sl, ALLEGRO_COLOR list[])
{
	assert(sl != NULL);
	for (int i = 0; i < STATUS_COLOR_MAX; i++)
		sl->colors[i] = list[i];
}


void
statusline_load_fonts(STATUSLINE *sl, FONT fontlist[])
{
	static FONT fi;
	static ALLEGRO_FONT *font;

	assert(sl != NULL);
	for (int i = 0; i < STATUS_FONT_MAX; i++) {
		fi = fontlist[i];
		font = al_load_font(fi.file, fi.size, fi.flags);
		assert(font != NULL);
		sl->fonts[i] = fi;

		if (sl->fonts[i].font != NULL) {
			al_destroy_font(sl->fonts[i].font);
			sl->fonts[i].font = NULL;
		}

		sl->fonts[i].font = font;
		sl->fonts[i].height = al_get_font_line_height(font);
	}
}


ALLEGRO_BITMAP *
statusline_bitmap(STATUSLINE *sl)
{
	assert(sl != NULL);
	return sl->b;
}


void
statusline_draw(STATUSLINE *sl)
{
	static int px, py;
	static FONT *F;

	assert(sl != NULL);
	px = sl->px;
	py = sl->py;
	F = &sl->fonts[STATUS_FONT_DEFAULT];

	al_set_target_bitmap(sl->b);
	al_clear_to_color(sl->colors[STATUS_COLOR_BACKGROUND]);

	al_draw_rectangle(px, 0, sl->w - px, sl->h - py,
		sl->colors[STATUS_COLOR_BORDER], 1);

	al_draw_ustr(F->font,
		sl->colors[STATUS_COLOR_FOREGROUND], F->px, F->py, 0, sl->us);

	if (sl->blink) {
		int x = al_get_ustr_width(F->font, sl->us)
			+ al_get_glyph_width(F->font, 0x0001);
		al_draw_filled_rectangle(x, F->py, x + sl->blink_w, F->height,
			sl->colors[STATUS_COLOR_BLINK]);
	}
}


void
statusline_type(STATUSLINE *sl, const ALLEGRO_USTR *us)
{
	assert(sl != NULL);
	al_ustr_truncate(sl->us, 0);

	if (us != NULL)
		al_ustr_append(sl->us, us);
}


void
statusline_blink(STATUSLINE *sl)
{
	assert(sl != NULL);
	sl->blink = !sl->blink;
}

void
statusline_noblink(STATUSLINE *sl)
{
	assert(sl != NULL);
	if (sl->blink)
		sl->blink = false;
}


void
statusline_resize(STATUSLINE *self, int w, int h)
{
	assert(self != NULL);

	self->w = w;
	self->h = h;

	if (self->b != NULL) {
		al_destroy_bitmap(self->b);
		self->b = NULL;
	}

	self->b = al_create_bitmap(self->w, self->h);
	assert(self->b);
}
