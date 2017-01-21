#include "typer.h"
#include "colors.h"
#include <stdlib.h>
#include <assert.h>


struct TYPER {
	ALLEGRO_BITMAP *b;
	int w, h;
	float x, y;
	ALLEGRO_USTR *text;
	struct {
		ALLEGRO_FONT *ptr;
		int size;
		int flags;
	} font;
};

TYPER *
typer_new(int w, int h)
{
	TYPER *t = malloc(sizeof(TYPER));

	assert(t != NULL);
	t->w = w;
	t->h = h;

	t->b = al_create_bitmap(t->w, t->h);
	assert(t->b != NULL);

	t->text = al_ustr_new("");
	assert(t->text != NULL);

	t->x = 5;
	t->y = (t->h / 2) + (t->font.size / 2);

	return t;
}

void
typer_destroy(TYPER *t)
{
	if (t == NULL)
		return;

	free(t);
}

ALLEGRO_BITMAP *
typer_bitmap(TYPER *t)
{
	assert(t != NULL);
	return t->b;
}

void
typer_type(TYPER *t, const char *cstr)
{
	assert(t != NULL);
	al_ustr_append_cstr(t->text, cstr);
}

void
typer_remove(TYPER *t)
{
	static ALLEGRO_USTR *u;

	assert(t != NULL);
	u = t->text;
	al_ustr_remove_chr(u, al_ustr_offset(u, -1));
}

void
typer_truncate(TYPER *t)
{
	assert(t != NULL);
	al_ustr_truncate(t->text, 0);
}

void
typer_draw(TYPER *t)
{
	static ALLEGRO_COLOR fg, bg;

	assert(t != NULL);
	bg = COLOR_NORMAL_BLACK;
	fg = COLOR_BRIGHT_GREEN;
	al_set_target_bitmap(t->b);
	al_clear_to_color(bg);	
	al_draw_ustr(t->font.ptr, fg, t->x, t->y, 0, t->text);
}

bool
typer_load_font(TYPER *t, FONT_INFO fi)
{
	static ALLEGRO_FONT *font;

	assert(t != NULL);
	//assert(id < TYPER_FONT_MAX);

	font = al_load_font(fi.file, fi.size, fi.flags);
	assert(font != NULL);

	if (font != NULL) {
		t->font.ptr = font;
		t->font.size = fi.size;
		t->font.flags = fi.flags;
		return true;
	}

	return false;
}

const ALLEGRO_USTR *
typer_get_text(TYPER *t)
{
	assert(t != NULL);
	return t->text;
}

void
typer_set_text(TYPER *t, const ALLEGRO_USTR *u)
{
	assert(t != NULL);
	assert(u != NULL);
	al_ustr_truncate(t->text, 0);
	al_ustr_append(t->text, u);
}

void
typer_set_text_cstr(TYPER *t, const char *str)
{
	assert(t != NULL);
	al_ustr_truncate(t->text, 0);
	al_ustr_append_cstr(t->text, str);
}
