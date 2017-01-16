#ifndef _AFV_FONTVIEWER_H__
#define _AFV_FONTVIEWER_H__

#include "engine.h"

typedef struct _afv_fontviewer FONTVIEWER;

FONTVIEWER * fontviewer_new(int width, int height);
void fontviewer_destroy(FONTVIEWER *fv);
bool fontviewer_load(FONTVIEWER *fv, const char *file);
void fontviewer_unload(FONTVIEWER *fv);
void fontviewer_draw(FONTVIEWER *fv);
ALLEGRO_BITMAP * fontviewer_bitmap(FONTVIEWER *fv);


#endif /* _AFV_FONTVIEWER_H__ */