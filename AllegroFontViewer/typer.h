#ifndef _AFV_TYPER_H__
#define _AFV_TYPER_H__

#include "engine.h"

typedef struct TYPER TYPER;

enum {
	TYPER_FONT_DEFAULT,
	TYPER_FONT_MAX
};


TYPER *
typer_new(int w, int h);

void
typer_destroy(TYPER *t);

ALLEGRO_BITMAP *
typer_bitmap(TYPER *t);

void
typer_type(TYPER *t, const char *cstr);

void
typer_remove(TYPER *t);

void
typer_truncate(TYPER *t);

void
typer_draw(TYPER *t);

bool
typer_load_font(TYPER *t, FONT fi);

const ALLEGRO_USTR *
typer_get_text(TYPER *t);

void
typer_set_text(TYPER *t, const ALLEGRO_USTR *u);

void
typer_set_text_cstr(TYPER *t, const char *str);


#endif /* _AFV_TYPER_H__ */