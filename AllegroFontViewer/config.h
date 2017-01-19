#ifndef _AFV_CONFIG_H__
#define _AFV_CONFIG_H__

#include "engine.h"
#include "FileBrowser.h"
#include "statusline.h"

enum {
	FONT_DEFAULT,
	FONT_BROWSER,
	FONT_STATUS,
	FONT_MAX
};

enum {
	CONFIG_AS_IS = 0,
	CONFIG_AS_INT,
	CONFIG_AS_FLOAT,
	CONFIG_AS_STRING
};

#define VIEWER_MAX_PRESETS 10

typedef struct CONFIG {
	struct {
		char *title;
		char *icon;
	} window;
	struct {
		int		w, h;
		float	rate;
		bool	vsync;
	} display;
	struct {
		ALLEGRO_COLOR	colors[FILEBROWSER_COLOR_MAX];
		FONT_INFO		fonts[FILEBROWSER_FONT_MAX];
		char			*startpath;
		int				scrollspeed;
	} browser;
	struct {
		int repeatrate;
	} keyboard;
	struct {
		ALLEGRO_COLOR	colors[STATUS_COLOR_MAX];
	} status;
	struct {
		ALLEGRO_USTR *presets[VIEWER_MAX_PRESETS]; /* FIXME */
	} viewer;

	FONT_INFO fonts[FONT_MAX];
} CONFIG;

CONFIG *
config_new(const char * const file);

void
config_destroy(CONFIG *c);

const char *
config_get_value(const char *section, const char *key, int type, void *out);

bool
parse_config(CONFIG *c);


#endif /* _AFV_CONFIG_H__ */
