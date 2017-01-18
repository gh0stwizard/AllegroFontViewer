#ifndef _AFV_FILEBROWSER_H__
#define _AFV_FILEBROWSER_H__

#include "engine.h"

typedef struct _afv_filebrowser FILEBROWSER;
typedef enum _afv_filebrowser_font_id FILEBROWSER_FONT_ID;
typedef enum _afv_filebrowser_color_id FILEBROWSER_COLOR_ID;

enum _afv_filebrowser_font_id {
	FILEBROWSER_FONT_DEFAULT,
	FILEBROWSER_FONT_DIR,
	FILEBROWSER_FONT_FILE,
	FILEBROWSER_FONT_MAX
};

enum _afv_filebrowser_color_id {
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
	FILEBROWSER_COLOR_MAX
};

enum {
	FILEBROWSER_DRAW_DIRS = 0x01,
	FILEBROWSER_DRAW_INFO = 0x02
};

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
filebrowser_load_font(FILEBROWSER *fb, FILEBROWSER_FONT_ID fontid,
	char const *file, int size, int flags);

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
filebrowser_draw_fileinfo(FILEBROWSER *fb);

void
filebrowser_draw_dirlist(FILEBROWSER *fb);

uint8_t
filebrowser_draw_mode(FILEBROWSER *fb, uint8_t mode);

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
