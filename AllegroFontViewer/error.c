#include "error.h"

#include <assert.h>


struct ERROR {
	ALLEGRO_BITMAP *b;
	int w, h;
	FONT fonts[ERROR_FONT_MAX];
	ALLEGRO_COLOR colors[ERROR_COLOR_MAX];
};


ERROR *
error_new(int w, int h)
{
	ERROR *error = al_malloc(sizeof(ERROR));
	assert(error != NULL);

	error->w = w;
	error->h = h;
	error->b = al_create_bitmap(error->w, error->h);
	assert(error->b);

	return error;
}


void
error_destroy(ERROR *err)
{
	if (err == NULL)
		return;

	if (err->b)
		al_destroy_bitmap(err->b);

	al_free(err);
}


void
error_draw(ERROR *err, ERROR_DATA *data)
{
	static ALLEGRO_FONT *font;
	static ALLEGRO_COLOR color_heading, color_message;
#if defined(_DEBUG)
	static ALLEGRO_COLOR color_dbg_text, color_dbg_file;
#endif
	static int center;

	const int x = 5;
	const int y = 20;

#define HEAD "ERROR"
	font = err->fonts[ERROR_FONT_DEFAULT].font;
	assert(font != NULL);
	center = err->w / 2 - al_get_text_width(font, HEAD) / 2;

	color_heading = err->colors[ERROR_COLOR_HEADING];
	color_message = err->colors[ERROR_COLOR_MESSAGE];
#if defined(_DEBUG)
	color_dbg_text = err->colors[ERROR_COLOR_DEBUG_TEXT];
	color_dbg_file = err->colors[ERROR_COLOR_DEBUG_FILE];
#endif

	al_set_target_bitmap(err->b);
	al_clear_to_color(err->colors[ERROR_COLOR_BACKGROUND]);
	al_draw_rectangle(2, 2, err->w - 2, err->h,
		err->colors[ERROR_COLOR_BORDER], 1);

	al_draw_text(font, color_heading, center, y, 0, HEAD);
	al_draw_ustr(font, color_message, x, y + 20, 0, data->message);
#undef HEAD

#if defined(_DEBUG)
	ALLEGRO_USTR *u = al_ustr_new("In function ");
	al_ustr_append(u, data->function);
	al_ustr_append_cstr(u, " at line ");
	al_ustr_append(u, data->line);
	al_ustr_append_cstr(u, ".");
	al_draw_ustr(font, color_dbg_text, x, y + 40, 0, u);
	al_draw_ustr(font, color_dbg_file, x, y + 60, 0, data->file);
	al_ustr_free(u);
#endif
}


ALLEGRO_BITMAP *
error_bitmap(ERROR *err)
{
	assert(err != NULL);
	return err->b;
}


void
error_set_colors(ERROR *err, ALLEGRO_COLOR list[])
{
	assert(err != NULL);
	for (int i = 0; i < ERROR_COLOR_MAX; i++)
		err->colors[i] = list[i];
}


void
error_load_fonts(ERROR *err, FONT fontlist[])
{
	static FONT fi;
	static ALLEGRO_FONT *font;

	assert(err != NULL);
	for (int i = 0; i < ERROR_FONT_MAX; i++) {
		fi = fontlist[i];
		font = al_load_font(fi.file, fi.size, fi.flags);
		assert(font != NULL);
		err->fonts[i] = fi;

		if (err->fonts[i].font != NULL) {
			al_destroy_font(err->fonts[i].font);
			err->fonts[i].font = NULL;
		}

		err->fonts[i].font = font;
		err->fonts[i].height = al_get_font_line_height(font);
	}
}


void
error_resize(ERROR *self, int w, int h)
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
