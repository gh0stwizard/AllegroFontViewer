#include "FontViewer.h"
#include "vector.h"
#include "colors.h"
#include "events.h"

#include <assert.h>


#define ERROR(viewer, ustr) (throw_error(viewer, __func__, __LINE__, ustr))

static void
throw_error(FONTVIEWER *fv, const char *function, int line, ALLEGRO_USTR *msg);

struct _afv_fontviewer {
	ALLEGRO_EVENT_SOURCE event_source;
	ALLEGRO_BITMAP *b;
	int w, h;
	int px, py;
	unsigned int minsize;
	unsigned int maxsize;
	VECTOR *fonts;
	ALLEGRO_PATH *path;
	struct { int onload; } flag;
	bool unload_first;
	bool drawed;
	ALLEGRO_USTR *text;
	ALLEGRO_COLOR colors[FONTVIEWER_COLOR_MAX];
};


FONTVIEWER *
fontviewer_new(int width, int height)
{
	static FONTVIEWER *fv;

	fv = al_calloc(1, sizeof(FONTVIEWER));
	assert(fv != NULL);

	fv->fonts = vector_new();
	assert(fv->fonts != NULL);

	fv->w = width;
	fv->h = height;
	fv->b = al_create_bitmap(fv->w, fv->h);
	assert(fv->b != NULL);

	fv->maxsize = 32;
	fv->minsize = 7;

	fv->unload_first = true;

	fv->flag.onload = 0;

	fv->px = 5;
	fv->py = 5;

	fv->text = al_ustr_new("One today is worth two tomorrows");
	assert(fv->text != NULL);

	al_init_user_event_source(&(fv->event_source));

	fv->colors[FONTVIEWER_COLOR_BACKGROUND] = COLOR_NORMAL_BLACK;
	fv->colors[FONTVIEWER_COLOR_FOREGROUND] = COLOR_BRIGHT_GREEN;

	return fv;
}

void
fontviewer_destroy(FONTVIEWER *fv)
{
	if (fv == NULL)
		return;

	if (fv->fonts != NULL) {
		vector_destroy(fv->fonts);
		fv->fonts = NULL;
	}

	if (fv->path != NULL) {
		al_destroy_path(fv->path);
		fv->path = NULL;
	}

	if (fv->b != NULL) {
		al_destroy_bitmap(fv->b);
		fv->b = NULL;
	}

	if (fv->text != NULL) {
		al_ustr_free(fv->text);
		fv->text = NULL;
	}

	al_destroy_user_event_source(&(fv->event_source));

	al_free(fv);
}

bool
fontviewer_load(FONTVIEWER *fv, const char *file)
{
	static VECTOR *V;
	static int flags;
	static bool success;
	static ALLEGRO_FONT *font;

	assert(fv != NULL);

	if (fv->unload_first)
		fontviewer_unload(fv);

	V = fv->fonts;
	flags = fv->flag.onload;
	success = true;
	for (int sz = fv->minsize, max = fv->maxsize; sz < max; sz++) {
		font = al_load_font(file, sz, flags);
		if (font == NULL) {
			fontviewer_unload(fv);
#if defined(_DEBUG)
			ERROR(fv, al_ustr_newf("Failed to load the font with size: %d.", sz));
#else
			ERROR(fv, al_ustr_new("Failed to load the font."));
#endif
			success = false;
			break;
		}
		else
			assert(vector_add(V, font));
	}

	if (fv->path != NULL)
		al_destroy_path(fv->path);
	fv->path = al_create_path(file);

	return success;
}

bool
fontviewer_load_path(FONTVIEWER *fv, const ALLEGRO_PATH *p)
{
	assert(fv != NULL);
	assert(p != NULL);
	return fontviewer_load(fv, al_path_cstr(p, ALLEGRO_NATIVE_PATH_SEP));
}

void
fontviewer_unload(FONTVIEWER *fv)
{
	static size_t i, count;
	static VECTOR *V;
	static ALLEGRO_FONT *F;

	assert(fv != NULL);
	V = fv->fonts;
	for (i = 0, count = vector_count(V); i < count; i++) {
		assert(vector_get(V, i, &F));
		al_destroy_font(F);
	}
	assert(vector_reset(V));
	fv->drawed = false;
}

void
fontviewer_draw(FONTVIEWER *fv)
{
	static ALLEGRO_FONT *F;
	static VECTOR *V;
	static size_t i, count, size;
	static ALLEGRO_USTR *U;
	static ALLEGRO_COLOR fg;
	static char *str;
	static int x, y, w, h, minsize, maxsize, px, py;

	assert(fv != NULL);

	V = fv->fonts;
	px = fv->px;
	py = fv->py;
	x = px;
	y = py;
	w = fv->w;
	h = fv->h;
	minsize = fv->minsize;
	maxsize = fv->maxsize;
	const int border_px = 2;
	const int border_py = 2;

	count = vector_count(V);
	if (count == 0) {
		fv->drawed = false;
		return;
	}
	fv->drawed = true;

	/* start drawing */
	al_set_target_bitmap(fv->b);
	al_clear_to_color(fv->colors[FONTVIEWER_COLOR_BACKGROUND]);
	al_reset_clipping_rectangle();
	al_draw_rectangle(border_px, border_py, w - border_px, h,
		fv->colors[FONTVIEWER_COLOR_BORDER], 1);

	fg = fv->colors[FONTVIEWER_COLOR_FOREGROUND];
	U = fv->text;
	al_set_clipping_rectangle(border_px, border_py, w - px - border_py, h - py);
	for (i = 0; i < count; i++) {
		assert(vector_get(V, i, &F));
		al_draw_ustr(F, fg, x, y, 0, U);
		y += (int)i + minsize + py;
		if (y + al_get_font_line_height(F) > h)
			break;
	}

	al_free(str);
}

ALLEGRO_BITMAP *
fontviewer_bitmap(FONTVIEWER *fv)
{
	assert(fv != NULL);
	return fv->b;
}

bool
fontviewer_is_drawn(FONTVIEWER *fv)
{
	assert(fv != NULL);
	return fv->drawed;
}

void
fontviewer_set_text(FONTVIEWER *fv, const ALLEGRO_USTR *str)
{
	assert(fv != NULL);
	assert(str != NULL);
	al_ustr_truncate(fv->text, 0);
	al_ustr_append(fv->text, str);
}


static void
event_dtor(ALLEGRO_USER_EVENT *ue)
{
	ALLEGRO_EVENT *e = (ALLEGRO_EVENT *)ue;
#if defined(_DEBUG)
	al_ustr_free((void *)e->user.data1);
#endif
	al_ustr_free((void *)e->user.data2);
	al_ustr_free((void *)e->user.data3);
	al_ustr_free((void *)e->user.data4);
}

static void
throw_error(FONTVIEWER *fv, const char *function, int line, ALLEGRO_USTR *msg)
{
	static ALLEGRO_EVENT ev;
	extern void event_dtor(ALLEGRO_USER_EVENT *e);

	assert(fv != NULL);
	assert(msg != NULL);
	ev.user.type = EVENT_TYPE_ERROR;
#if defined(_DEBUG)
	ev.user.data1 = (intptr_t)al_ustr_new(__FILE__);
#endif
	ev.user.data2 = (intptr_t)al_ustr_new(function);
	ev.user.data3 = (intptr_t)al_ustr_newf("%d", line);
	ev.user.data4 = (intptr_t)msg;

	al_emit_user_event((ALLEGRO_EVENT_SOURCE *)fv, &ev, event_dtor);
}

void
fontviewer_set_colors(FONTVIEWER *fv, ALLEGRO_COLOR list[])
{
	assert(fv != NULL);
	for (int i = 0; i < FONTVIEWER_COLOR_MAX; i++)
		fv->colors[i] = list[i];
}

int
fontviewer_get_font_size_mouse(FONTVIEWER *fv, int mx, int my)
{
	static int min, py, y;
	static size_t i, count;
	static VECTOR *V;
	static ALLEGRO_FONT *F;

	assert(fv != NULL);

	if (fv->drawed == false)
		return -1;
	/*
	 * F(y, maxHeight)		f->py + ((int)i + fv->minsize + fv->py, i>0)
	 */
	V = fv->fonts;
	y = py = fv->py;
	min = fv->minsize;
	count = vector_count(V);

	for (i = 0; i < count; i++) {
		if (my <= py)
			return min;

		if (my >= y) {
			y += (int)i + min + py;
			if (my < y)
				return ((int)i + min);
		}

		assert(vector_get(V, i, &F));
		if (y + al_get_font_line_height(F) > fv->h)
			break;
	}

	return -1;
}

void
fontviewer_set_font_size_limits(FONTVIEWER *fv, unsigned int min, 
	unsigned int max)
{
	assert(fv != NULL);
	fv->minsize = min;
	fv->maxsize = max;
}

FONT_ATTR *
fontviewer_get_attr_by_size(FONTVIEWER *fv, int size)
{
	static ALLEGRO_FONT *font;
	static size_t i, count;
	static FONT_ATTR *attr;

	assert(fv != NULL);
	count = vector_count(fv->fonts);
	i = size - fv->minsize;

	if (i < count) {
		assert(vector_get(fv->fonts, i, &font));
		attr = al_malloc(sizeof(FONT_ATTR));
		assert(attr != NULL);
		attr->height = al_get_font_line_height(font);
		attr->ascent = al_get_font_ascent(font);
		attr->descent = al_get_font_descent(font);
		/* TODO: al_get_text_dimensions() ??? */
		return attr;
	}
	else
		return NULL;
}


void
fontviewer_resize(FONTVIEWER *self, int w, int h)
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
