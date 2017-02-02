#ifndef _AFV_CONFIG_H__
#define _AFV_CONFIG_H__

#include "engine.h"
#include "FileBrowser.h"
#include "FontViewer.h"
#include "statusline.h"
#include "helpmenu.h"
#include "typer.h"
#include "error.h"


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
		char *icon;		/* file path to icon file */
		int x, y;		/* window position */
		int w, h;
		bool maximize;	/* is the window maximized? */
	} window;
	struct {
		int		w, h;
		float	rate;
		bool	vsync;
		bool	fullscreen;
		bool	fswindowed;
	} display;
	struct {
		ALLEGRO_COLOR	colors[FILEBROWSER_COLOR_MAX];
		FONT			fonts[FILEBROWSER_FONT_MAX];
		char			*startpath;
		int				scrollspeed;
	} browser;
	struct {
		int repeatrate;
	} keyboard;
	struct {
		ALLEGRO_COLOR	colors[STATUS_COLOR_MAX];
		FONT			fonts[STATUS_FONT_MAX];
		float			timeout;
		int				height;
	} status;
	struct {
		int minsize, maxsize;
		ALLEGRO_USTR	*presets[VIEWER_MAX_PRESETS];
		ALLEGRO_COLOR	colors[FONTVIEWER_COLOR_MAX];
	} viewer;
	struct {
		FONT			fonts[TYPER_FONT_MAX];
		float			blink_period;
	} typer;
	struct {
		ALLEGRO_COLOR	colors[HELP_MENU_COLOR_MAX];
		FONT			fonts[HELP_MENU_FONT_MAX];
	} help;
	struct {
		ALLEGRO_COLOR	colors[ERROR_COLOR_MAX];
		FONT			fonts[ERROR_FONT_MAX];
	} error;

} CONFIG;

CONFIG *
config_new(const char * const file);

void
config_destroy(CONFIG *c);

bool
config_get_value(const char *section, const char *key, int type, void *out);

bool
parse_config(CONFIG *c);

bool
config_save(CONFIG *c);

void
config_update(const char *section, const char *k, const char *v);


#endif /* _AFV_CONFIG_H__ */
