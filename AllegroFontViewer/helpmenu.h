#ifndef _AFV_HELPMENU_H__
#define _AFV_HELPMENU_H__

#include "engine.h"

typedef struct HELP_MENU HELP_MENU;

enum {
	HELP_MENU_COLOR_BACKGROUND,
	HELP_MENU_COLOR_FOREGROUND,
	HELP_MENU_COLOR_BORDER,
	HELP_MENU_COLOR_MAX
};

enum {
	HELP_MENU_FONT_DEFAULT,
	HELP_MENU_FONT_MAX
};


extern HELP_MENU *
helpmenu_new(int width, int height);

extern void
helpmenu_destroy(HELP_MENU *h);

extern ALLEGRO_BITMAP *
helpmenu_bitmap(HELP_MENU *h);

extern void
helpmenu_draw(HELP_MENU *help);

extern void
helpmenu_set_colors(HELP_MENU *h, ALLEGRO_COLOR list[]);

extern void
helpmenu_load_fonts(HELP_MENU *help, FONT fontlist[]);

extern void
helpmenu_resize(HELP_MENU *self, int w, int h);


#endif /* _AFV_HELPMENU_H__ */
