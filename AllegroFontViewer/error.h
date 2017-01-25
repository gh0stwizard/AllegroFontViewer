#ifndef _AFV_ERROR_H__
#define _AFV_ERROR_H__

#include "engine.h"


typedef struct ERROR ERROR;


typedef struct _afv_error_data {
	ALLEGRO_USTR *file;
	ALLEGRO_USTR *function;
	ALLEGRO_USTR *line;
	ALLEGRO_USTR *message;
} ERROR_DATA;


enum {
	ERROR_FONT_DEFAULT,
	ERROR_FONT_MAX
};


enum {
	ERROR_COLOR_BACKGROUND,
	ERROR_COLOR_BORDER,
	ERROR_COLOR_HEADING,
	ERROR_COLOR_MESSAGE,
	ERROR_COLOR_DEBUG_TEXT,
	ERROR_COLOR_DEBUG_FILE,
	ERROR_COLOR_MAX
};


ERROR *
error_new(int w, int h);

void
error_destroy(ERROR *err);

void
error_draw(ERROR *err, ERROR_DATA *data);

ALLEGRO_BITMAP *
error_bitmap(ERROR *err);

void
error_set_colors(ERROR *err, ALLEGRO_COLOR list[]);

void
error_load_fonts(ERROR *err, FONT fontlist[]);

void
error_resize(ERROR *self, int w, int h);


#endif /* _AFV_ERROR_H__ */
