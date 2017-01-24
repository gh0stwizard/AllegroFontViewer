#ifndef _AFV_FILEBROWSER_H__
#define _AFV_FILEBROWSER_H__

#include "engine.h"


typedef struct FILEBROWSER FILEBROWSER;


typedef enum FILEBROWSER_FONT_ID {
	FILEBROWSER_FONT_DEFAULT,
	FILEBROWSER_FONT_MAX
} FILEBROWSER_FONT_ID;


typedef enum FILEBROWSER_COLOR_ID {
	FILEBROWSER_COLOR_BACKGROUND,
	FILEBROWSER_COLOR_FOREGROUND,
	FILEBROWSER_COLOR_DIR_BG,
	FILEBROWSER_COLOR_DIR_FG,
	FILEBROWSER_COLOR_FILE_BG,
	FILEBROWSER_COLOR_FILE_FG,
	FILEBROWSER_COLOR_SELECT_BG,
	FILEBROWSER_COLOR_SELECT_FG,
	FILEBROWSER_COLOR_FONT_TTF_BG,
	FILEBROWSER_COLOR_FONT_TTF_FG,
	FILEBROWSER_COLOR_FONT_OTF_BG,
	FILEBROWSER_COLOR_FONT_OTF_FG,
	FILEBROWSER_COLOR_BORDER,
	FILEBROWSER_COLOR_MAX
} FILEBROWSER_COLOR_ID;


enum {
	FILEBROWSER_HOOK_SORT_DIRS = 0x01,
	FILEBROWSER_HOOK_SORT_FILE = 0x02
};


FILEBROWSER *
filebrowser_new(int width, int height);

void
filebrowser_destroy(FILEBROWSER * fb);

ALLEGRO_BITMAP *
filebrowser_bitmap(FILEBROWSER *fb);

bool
filebrowser_load_fonts(FILEBROWSER *fb, FONT fontlist[]);

bool
filebrowser_set_font_padding(FILEBROWSER *fb, FILEBROWSER_FONT_ID fontid,
	int px, int py);

bool
filebrowser_browse_path(FILEBROWSER *fb, const char *path);

bool
filebrowser_browse_selected(FILEBROWSER *fb);

bool
filebrowser_browse_parent(FILEBROWSER *fb);

bool
filebrowser_change_path(FILEBROWSER *fb, ALLEGRO_PATH *np);

bool
filebrowser_select_prev(FILEBROWSER *fb);

bool
filebrowser_select_prev_items(FILEBROWSER *fb, int percent);

bool
filebrowser_select_next(FILEBROWSER *fb);

bool
filebrowser_select_next_items(FILEBROWSER *fb, int percent);

void
filebrowser_draw(FILEBROWSER *fb);

void
filebrowser_set_hook(FILEBROWSER *fb, uint8_t id, void(*hook)(FILEBROWSER *));

void
filebrowser_sort(FILEBROWSER *fb, uint8_t type,
	int(*cmp)(const void *, const void *));

ALLEGRO_PATH *
filebrowser_get_selected_path(FILEBROWSER *fb);

void
filebrowser_set_colors(FILEBROWSER *fb, ALLEGRO_COLOR list[]);


#endif /* _AFV_FILEBROWSER_H__ */
