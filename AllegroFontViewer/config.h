#ifndef _AFV_CONFIG_H__
#define _AFV_CONFIG_H__

#include "engine.h"
#include "FileBrowser.h"

enum {
	FONT_DEFAULT,
	FONT_MAX
};

typedef struct _afv_font_info {
	char *file;
	int size;
	int flags;
} FONT_INFO;

typedef struct _afv_configuration CONFIG;
struct _afv_configuration {
	struct {
		char *title;
		char *icon;
	} window;
	struct {
		int w, h;
		float rate;
		bool vsync;
	} display;
	FONT_INFO fonts[FONT_MAX];
	struct {
		ALLEGRO_COLOR colors[FILEBROWSER_COLOR_MAX];
		FONT_INFO fonts[FILEBROWSER_FONT_MAX];
	} browser;
};

enum {
	CONFIG_AS_IS = 0,
	CONFIG_AS_INT,
	CONFIG_AS_FLOAT,
	CONFIG_AS_STRING
};

CONFIG *
config_new(const char * const file);

void
config_destroy(CONFIG *c);

const char *
config_get_value(const char *section, const char *key, int type, void *out);

bool
parse_config(CONFIG *c);


#endif /* _AFV_CONFIG_H__ */
