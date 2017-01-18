#include "FontViewer.h"
#include "vector.h"
#include "colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


struct _afv_fontviewer {
	ALLEGRO_BITMAP *b;
	int w, h;
	int px, py;
	uint8_t minsize;
	uint8_t maxsize;
	VECTOR *fonts;
	ALLEGRO_PATH *path;
	struct { int onload; } flag;
	bool unload_first;
	bool drawed;
};

FONTVIEWER *
fontviewer_new(int width, int height)
{
	static FONTVIEWER *fv;

	fv = calloc(1, sizeof(FONTVIEWER));
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

	free(fv);
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

	//fprintf(stderr, "loading %s\n", file);

	V = fv->fonts;
	flags = fv->flag.onload;
	success = true;
	for (int sz = fv->minsize, max = fv->maxsize; sz < max; sz++) {
		font = al_load_font(file, sz, flags);
		if (font == NULL) {
			fprintf(stderr, "failed to load font with size %d\n", sz);
			success = false;
			break;
		}
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
	const char *path = al_path_cstr(p, ALLEGRO_NATIVE_PATH_SEP);
	return fontviewer_load(fv, path);
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
}

void
fontviewer_draw(FONTVIEWER *fv)
{
	static ALLEGRO_FONT *F;
	static VECTOR *V;
	static size_t i, count, size;
	static ALLEGRO_USTR *U;
	static ALLEGRO_COLOR bg, fg;
	static char *str;
	static int x, y, maxH, minsize, maxsize, px, py, z;

	assert(fv != NULL);

	V = fv->fonts;

	bg = COLOR_BRIGHT_BLACK;
	fg = COLOR_BRIGHT_YELLOW;

	px = fv->px;
	py = fv->py;

	x = px;
	y = py;
	z = 0;

	maxH = fv->h - fv->py;
	minsize = fv->minsize;
	maxsize = fv->maxsize;

	count = vector_count(V);
	if (count == 0) {
		fv->drawed = false;
		return;
	}
	fv->drawed = true;

	/* start drawing */
	al_set_target_bitmap(fv->b);
	al_clear_to_color(bg);

	U = al_ustr_new("");
#define NOTE "One today is worth two tomorrows"
	size = (strlen(NOTE) + 1);
	str = malloc(sizeof(char) * size);
	assert(str != NULL);
	for (i = 0; i < count && z < maxH; i++) {
		if (vector_get(V, i, &F)) {
			al_ustr_truncate(U, 0);
#if defined(_WIN32) || defined (_WIN64)
			sprintf_s(str, (const size_t)size, NOTE);
#else
			sprintf(str, NOTE);
#endif
			al_ustr_append_cstr(U, str);
			al_draw_ustr(F, fg, x, y, 0, U);
			y += (int)i + minsize + py;
			z = y + (int)i + minsize + py;
		}
		else {
			fprintf(stderr, "failed to load font %zu of %zu\n", i, count);
			break;
		}
	}
#undef NOTE
	al_ustr_free(U);
	free(str);
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
