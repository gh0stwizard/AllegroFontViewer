#ifndef _AFV_HELPMENU_H__
#define _AFV_HELPMENU_H__

#include "engine.h"

typedef struct HELP_MENU HELP_MENU;

enum {
	HELP_MENU_COLOR_BACKGROUND,
	HELP_MENU_COLOR_FOREGROUND,
	HELP_MENU_COLOR_MAX
};

enum {
	HELP_MENU_FONT_DEFAULT,
	HELP_MENU_FONT_MAX
};


HELP_MENU *
helpmenu_new(int width, int height);

void
helpmenu_destroy(HELP_MENU *h);

ALLEGRO_BITMAP *
helpmenu_bitmap(HELP_MENU *h);

void
helpmenu_draw(HELP_MENU *h);

void
helpmenu_set_colors(HELP_MENU *h, ALLEGRO_COLOR list[]);

bool
helpmenu_load_font(HELP_MENU *h, int id, FONT_INFO fi);


#endif /* _AFV_HELPMENU_H__ */
