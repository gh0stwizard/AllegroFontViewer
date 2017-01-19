#ifndef _AFV_FONTVIEWER_H__
#define _AFV_FONTVIEWER_H__

#include "engine.h"

typedef struct _afv_fontviewer FONTVIEWER;

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


#endif /* _AFV_FONTVIEWER_H__ */