#ifndef _AFV_FONTVIEWER_H__
#define _AFV_FONTVIEWER_H__

#include "engine.h"

typedef struct _afv_fontviewer FONTVIEWER;
typedef struct FONT_ATTR {
	int height;
	int ascent;
	int descent;
} FONT_ATTR;

enum {
	FONTVIEWER_COLOR_BACKGROUND,
	FONTVIEWER_COLOR_FOREGROUND,
	FONTVIEWER_COLOR_BORDER,
	FONTVIEWER_COLOR_MAX
};

FONTVIEWER *
fontviewer_new(int width, int height);

void
fontviewer_destroy(FONTVIEWER *fv);

bool
fontviewer_load(FONTVIEWER *fv, const char *file);

bool
fontviewer_load_path(FONTVIEWER *fv, const ALLEGRO_PATH *p);

void
fontviewer_unload(FONTVIEWER *fv);

void
fontviewer_draw(FONTVIEWER *fv);

ALLEGRO_BITMAP *
fontviewer_bitmap(FONTVIEWER *fv);

bool
fontviewer_is_drawn(FONTVIEWER *fv);

void
fontviewer_set_text(FONTVIEWER *fv, const ALLEGRO_USTR *str);

void
fontviewer_set_colors(FONTVIEWER *fv, ALLEGRO_COLOR list[]);

int
fontviewer_get_font_size_mouse(FONTVIEWER *fv, int x, int y);

void
fontviewer_set_font_size_limits(FONTVIEWER *fv, unsigned int min,
	unsigned int max);

FONT_ATTR *
fontviewer_get_attr_by_size(FONTVIEWER *fv, int size);

void
fontviewer_resize(FONTVIEWER *self, int w, int h);


#endif /* _AFV_FONTVIEWER_H__ */
