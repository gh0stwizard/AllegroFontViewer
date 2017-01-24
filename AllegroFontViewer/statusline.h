#ifndef _AFV_STATUSLINE_H__
#define _AFV_STATUSLINE_H__

#include "engine.h"
#include "events.h"


enum {
	STATUS_FONT_DEFAULT,
	STATUS_FONT_MAX
};


enum {
	STATUS_COLOR_BACKGROUND,
	STATUS_COLOR_FOREGROUND,
	STATUS_COLOR_BORDER,
	STATUS_COLOR_BLINK,
	STATUS_COLOR_MAX
};

typedef struct STATUSLINE STATUSLINE;

STATUSLINE *
statusline_new(int w, int h, int px, int py);

void
statusline_destroy(STATUSLINE *sl);

void
statusline_set_colors(STATUSLINE *sl, ALLEGRO_COLOR list[]);

void
statusline_load_fonts(STATUSLINE *sl, FONT fontlist[]);

ALLEGRO_BITMAP *
statusline_bitmap(STATUSLINE *sl);

void
statusline_draw(STATUSLINE *sl, const ALLEGRO_USTR *us);

void
statusline_blink(STATUSLINE *sl);

void
statusline_noblink(STATUSLINE *sl);


#endif /* _AFV_STATUSLINE_H__ */
