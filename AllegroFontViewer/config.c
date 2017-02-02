#include "config.h"

#include <assert.h>
#include <errno.h>

#if defined(_DEBUG)
#include <stdio.h>
#endif


#define DEFAULT_CONFIG_FILE	"defaults.ini"
#define USER_CONFIG_FILE	"settings.ini"
static ALLEGRO_CONFIG *defaults, *current;


/* pre-defined default values in case if there is no a conf. file */
#define DISPLAY_W 640
#define DISPLAY_H 480
#define DISPLAY_FPS 60.0
#define ENABLE_VSYNC true
#define FULLSCREEN false
#define FS_WINDOWED false

#define WINDOW_TITLE "Allegro Font Viewer"

#define TYPER_BLINK_PERIOD 0.8

#define BROWSER_STARTPATH "."
#define BROWSER_SCROLLSPEED 50

#define STATUSLINE_PERIOD 10.0
#define STATUSLINE_HEIGHT 20

#define KB_REPEAT_RATE 100

#define VIEWER_MINSIZE 7
#define VIEWER_MAXSIZE 32


CONFIG *
config_new(const char * const file)
{
#ifndef _DEBUG
	static ALLEGRO_PATH *p;
#endif

	CONFIG *cfg = al_calloc(1, sizeof(CONFIG));
	assert(cfg != NULL);

	if (file == NULL) {
#ifndef _DEBUG
		p = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
		al_set_path_filename(p, DEFAULT_CONFIG_FILE);
		defaults = al_load_config_file(al_path_cstr(p,
			ALLEGRO_NATIVE_PATH_SEP));
#else
		defaults = al_load_config_file(DEFAULT_CONFIG_FILE);
#endif
		assert(defaults != NULL);

		/* TODO: replace assert to this...
		if (defaults == NULL)
			defaults = al_create_config();
		*/

#ifndef _DEBUG
		al_set_path_filename(p, USER_CONFIG_FILE);
		current = al_load_config_file(al_path_cstr(p,
			ALLEGRO_NATIVE_PATH_SEP));
		al_destroy_path(p);
#else
		current = al_load_config_file(USER_CONFIG_FILE);
#endif
	}
	else
		current = al_load_config_file(file);

	if (current == NULL)
		current = al_create_config();

	assert(current != NULL);

	parse_config(cfg);

	return cfg;
}


void
config_destroy(CONFIG *c)
{
	if (c == NULL)
		return;

	if (c->window.title)
		al_free(c->window.title);

	if (c->window.icon)
		al_free(c->window.icon);

	for (int i = 0; i < FILEBROWSER_FONT_MAX; i++) {
		if (c->browser.fonts[i].file != NULL) {
			al_free(c->browser.fonts[i].file);
			c->browser.fonts[i].file = NULL;
		}
	}

	if (c->browser.startpath != NULL)
		al_free(c->browser.startpath);

	for (int i = 0; i < VIEWER_MAX_PRESETS; i++) {
		ALLEGRO_USTR *u = c->viewer.presets[i];
		if (u != NULL)
			al_ustr_free(u);
	}

	al_free(c);
}


static const char *
_get_value(const char *section, const char *key)
{
	static const char *retval;

	retval = al_get_config_value(current, section, key);

	if (retval == NULL && defaults != NULL)
		retval = al_get_config_value(defaults, section, key);

	return retval;
}


#if defined(ALLEGRO_WINDOWS)
#define ATOI(x) (atoi(x))
#define ATOF(x) (atof(x))
#define STRNCPY(a, b, c, d) (strncpy_s(a, b, c, d))
#else
#define ATOI(x) (strtol(x, NULL, 10))
#define ATOF(x) (strtod(x, NULL))
#define STRNCPY(a, b, c, d) \
do { \
    len = strlen(c); \
    strncpy(a, c, ((len >= d) ? d : len)); \
} while (0)
#endif

#define STR_BUFSIZE 1023

bool
config_get_value(const char *section, const char *key, int type, void *out)
{
	static int i;
	static double d;
	static size_t len, size;
	static char *dst;
	static const char *src;
	static bool found;


	src = _get_value(section, key);
	found = true;

	if (src == NULL) {
		src = "";
		found = false;
	}

	errno = 0;

	switch (type) {
	case CONFIG_AS_INT:
		i = ATOI(src);
		assert(errno != ERANGE);
		*(int32_t *)out = i;
		break;

	case CONFIG_AS_FLOAT:
		d = ATOF(src);
		assert(errno != ERANGE);
		*(float *)out = (float)d; /*TODO: approximation */
		break;

	case CONFIG_AS_STRING:
		size = sizeof(char) * (STR_BUFSIZE + 1);
		dst = (char *)al_malloc(size);
		assert(dst != NULL);
		len = strlen(src);
		STRNCPY(dst, size, src, STR_BUFSIZE);
		*(char **)out = dst;
		break;
	}

	return found;
}

bool
parse_config(CONFIG *c)
{
	static int inum;
	static float flt;
	static char *str;

	/* FIXME: auto generate this code or auto-gen structure! */

	/* window */
	if (config_get_value("window", "title", CONFIG_AS_STRING, &str))
		c->window.title = str;
	else
		c->window.title = WINDOW_TITLE;

	if (config_get_value("window", "icon", CONFIG_AS_STRING, &str))
		c->window.icon = str;

	if (config_get_value("window", "x", CONFIG_AS_INT, &inum))
		c->window.x = inum;
	else
		c->window.x = INT_MAX;

	if (config_get_value("window", "y", CONFIG_AS_INT, &inum))
		c->window.y = inum;
	else
		c->window.y = INT_MAX;

	if (config_get_value("window", "maximize", CONFIG_AS_INT, &inum))
		c->window.maximize = !(inum == 0);
	else
		c->window.maximize = false;

	if (config_get_value("window", "width", CONFIG_AS_INT, &inum))
		c->window.w = inum;
	else
		c->window.w = INT_MAX;

	if (config_get_value("window", "height", CONFIG_AS_INT, &inum))
		c->window.h = inum;
	else
		c->window.h = INT_MAX;


	/* display */
	if (config_get_value("display", "width", CONFIG_AS_INT, &inum))
		c->display.w = inum;
	else
		c->display.w = DISPLAY_W;

	if (config_get_value("display", "height", CONFIG_AS_INT, &inum))
		c->display.h = inum;
	else
		c->display.h = DISPLAY_H;

	if (config_get_value("display", "framerate", CONFIG_AS_FLOAT, &flt))
		c->display.rate = flt;
	else
		c->display.rate = DISPLAY_FPS;

	if (config_get_value("display", "vsync", CONFIG_AS_INT, &inum))
		c->display.vsync = !(inum == 0);
	else
		c->display.vsync = ENABLE_VSYNC;

	if (config_get_value("display", "fullscreen", CONFIG_AS_INT, &inum))
		c->display.fullscreen = !(inum == 0);
	else
		c->display.fullscreen = FULLSCREEN;

	if (config_get_value("display", "fswindowed", CONFIG_AS_INT, &inum))
		c->display.fswindowed = !(inum == 0);
	else
		c->display.fswindowed = FS_WINDOWED;

	/* status */
	if (config_get_value("status", "timeout", CONFIG_AS_FLOAT, &flt))
		c->status.timeout = flt;
	else
		c->status.timeout = STATUSLINE_PERIOD;

	if (config_get_value("status", "height", CONFIG_AS_INT, &inum))
		c->status.height = inum;
	else
		c->status.height = STATUSLINE_HEIGHT;

	if (config_get_value("colors:status", "background", CONFIG_AS_STRING, &str))
		c->status.colors[STATUS_COLOR_BACKGROUND] = al_color_name(str);
	else
		c->status.colors[STATUS_COLOR_BACKGROUND] = al_color_name("black");

	if (config_get_value("colors:status", "foreground", CONFIG_AS_STRING, &str))
		c->status.colors[STATUS_COLOR_FOREGROUND] = al_color_name(str);
	else
		c->status.colors[STATUS_COLOR_FOREGROUND] = al_color_name("gold");

	if (config_get_value("colors:status", "border", CONFIG_AS_STRING, &str))
		c->status.colors[STATUS_COLOR_BORDER] = al_color_name(str);
	else
		c->status.colors[STATUS_COLOR_BORDER] = al_color_name("slategray");

	if (config_get_value("colors:status", "blink", CONFIG_AS_STRING, &str))
		c->status.colors[STATUS_COLOR_BLINK] = al_color_name(str);
	else
		c->status.colors[STATUS_COLOR_BLINK] = al_color_name("lightblue");

	if (config_get_value("font:status", "file", CONFIG_AS_STRING, &str))
		c->status.fonts[STATUS_FONT_DEFAULT].file = str;

	if (config_get_value("font:status", "size", CONFIG_AS_INT, &inum))
		c->status.fonts[STATUS_FONT_DEFAULT].size = inum;

	for (int i = 0; i < STATUS_FONT_MAX; i++)
		c->status.fonts[i].flags = 0;

	if (config_get_value("font:status", "px", CONFIG_AS_INT, &inum))
		c->status.fonts[STATUS_FONT_DEFAULT].px = inum;

	if (config_get_value("font:status", "py", CONFIG_AS_INT, &inum))
		c->status.fonts[STATUS_FONT_DEFAULT].py = inum;


	/* browser */
	if (config_get_value("browser", "startpath", CONFIG_AS_STRING, &str))
		c->browser.startpath = str;
	else
		c->browser.startpath = BROWSER_STARTPATH;

	if (config_get_value("browser", "scrollspeed", CONFIG_AS_INT, &inum))
		c->browser.scrollspeed = inum;
	else
		c->browser.scrollspeed = BROWSER_SCROLLSPEED;

	config_get_value("colors:browser", "background", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_BACKGROUND] = al_color_name(str);
	config_get_value("colors:browser", "foreground", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FOREGROUND] = al_color_name(str);
	config_get_value("colors:browser", "border", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_BORDER] = al_color_name(str);

	config_get_value("colors:browser:dirs", "background", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_DIR_BG] = al_color_name(str);
	config_get_value("colors:browser:dirs", "foreground", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_DIR_FG] = al_color_name(str);

	config_get_value("colors:browser:file", "background", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FILE_BG] = al_color_name(str);
	config_get_value("colors:browser:file", "foreground", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FILE_FG] = al_color_name(str);

	config_get_value("colors:browser:selected", "background", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_SELECT_BG] = al_color_name(str);
	config_get_value("colors:browser:selected", "foreground", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_SELECT_FG] = al_color_name(str);

	config_get_value("colors:browser:ttf", "background", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FONT_TTF_BG] = al_color_name(str);
	config_get_value("colors:browser:ttf", "foreground", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FONT_TTF_FG] = al_color_name(str);

	config_get_value("colors:browser:otf", "background", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FONT_OTF_BG] = al_color_name(str);
	config_get_value("colors:browser:otf", "foreground", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FONT_OTF_FG] = al_color_name(str);

	config_get_value("colors:browser:ttc", "background", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FONT_TTC_BG] = al_color_name(str);
	config_get_value("colors:browser:ttc", "foreground", CONFIG_AS_STRING, &str);
	c->browser.colors[FILEBROWSER_COLOR_FONT_TTC_FG] = al_color_name(str);

	config_get_value("font:browser", "file", CONFIG_AS_STRING, &str);
	c->browser.fonts[FILEBROWSER_FONT_DEFAULT].file = str;
	config_get_value("font:browser", "size", CONFIG_AS_INT, &inum);
	c->browser.fonts[FILEBROWSER_FONT_DEFAULT].size = inum;

	for (int i = 0; i < FILEBROWSER_FONT_MAX; i++)
		c->browser.fonts[i].flags = 0;

	if (config_get_value("font:browser", "px", CONFIG_AS_INT, &inum))
		c->browser.fonts[FILEBROWSER_FONT_DEFAULT].px = inum;

	if (config_get_value("font:browser", "py", CONFIG_AS_INT, &inum))
		c->browser.fonts[FILEBROWSER_FONT_DEFAULT].py = inum;


	/* keyboard */
	if (config_get_value("keyboard", "repeatrate", CONFIG_AS_INT, &inum))
		c->keyboard.repeatrate = inum;
	else
		c->keyboard.repeatrate = KB_REPEAT_RATE;


	/* viewer */
	assert(VIEWER_MAX_PRESETS <= 10);
	for (int i = 0; i < VIEWER_MAX_PRESETS; i++) {
		/* XXX */
		char preset_str[8] = { 'p', 'r', 'e', 's', 'e', 't', i + 48, '\0' };
		if (config_get_value("viewer", preset_str, CONFIG_AS_STRING, &str))
			c->viewer.presets[i] = al_ustr_new(str);
		else
			c->viewer.presets[i] = al_ustr_new("");
	}

	if (config_get_value("viewer", "minsize", CONFIG_AS_INT, &inum))
		c->viewer.minsize = inum;
	else
		c->viewer.minsize = VIEWER_MINSIZE;

	if (config_get_value("viewer", "maxsize", CONFIG_AS_INT, &inum))
		c->viewer.maxsize = inum;
	else
		c->viewer.maxsize = VIEWER_MAXSIZE;

	config_get_value("colors:viewer", "background", CONFIG_AS_STRING, &str);
	c->viewer.colors[FONTVIEWER_COLOR_BACKGROUND] = al_color_name(str);
	config_get_value("colors:viewer", "foreground", CONFIG_AS_STRING, &str);
	c->viewer.colors[FONTVIEWER_COLOR_FOREGROUND] = al_color_name(str);
	config_get_value("colors:viewer", "border", CONFIG_AS_STRING, &str);
	c->viewer.colors[FONTVIEWER_COLOR_BORDER] = al_color_name(str);


	/* typer */
	if (config_get_value("font:typer", "file", CONFIG_AS_STRING, &str))
		c->typer.fonts[TYPER_FONT_DEFAULT].file = str;

	if (config_get_value("font:typer", "size", CONFIG_AS_INT, &inum))
		c->typer.fonts[TYPER_FONT_DEFAULT].size = inum;

	for (int i = 0; i < TYPER_FONT_MAX; i++)
		c->typer.fonts[i].flags = 0;

	if (config_get_value("typer", "blinkperiod", CONFIG_AS_FLOAT, &flt))
		c->typer.blink_period = flt;
	else
		c->typer.blink_period = TYPER_BLINK_PERIOD;


	/* help */
	config_get_value("colors:help", "background", CONFIG_AS_STRING, &str);
	c->help.colors[HELP_MENU_COLOR_BACKGROUND] = al_color_name(str);
	config_get_value("colors:help", "foreground", CONFIG_AS_STRING, &str);
	c->help.colors[HELP_MENU_COLOR_FOREGROUND] = al_color_name(str);
	config_get_value("colors:help", "border", CONFIG_AS_STRING, &str);
	c->help.colors[HELP_MENU_COLOR_BORDER] = al_color_name(str);

	if (config_get_value("font:help", "file", CONFIG_AS_STRING, &str))
		c->help.fonts[HELP_MENU_FONT_DEFAULT].file = str;

	if (config_get_value("font:help", "size", CONFIG_AS_INT, &inum))
		c->help.fonts[HELP_MENU_FONT_DEFAULT].size = inum;

	for (int i = 0; i < HELP_MENU_FONT_MAX; i++)
		c->help.fonts[i].flags = 0;


	/* error */
	config_get_value("colors:error", "background", CONFIG_AS_STRING, &str);
	c->error.colors[ERROR_COLOR_BACKGROUND] = al_color_name(str);
	config_get_value("colors:error", "border", CONFIG_AS_STRING, &str);
	c->error.colors[ERROR_COLOR_BORDER] = al_color_name(str);
	config_get_value("colors:error", "heading", CONFIG_AS_STRING, &str);
	c->error.colors[ERROR_COLOR_HEADING] = al_color_name(str);
	config_get_value("colors:error", "message", CONFIG_AS_STRING, &str);
	c->error.colors[ERROR_COLOR_MESSAGE] = al_color_name(str);
	config_get_value("colors:error", "debugtext", CONFIG_AS_STRING, &str);
	c->error.colors[ERROR_COLOR_DEBUG_TEXT] = al_color_name(str);
	config_get_value("colors:error", "debugfile", CONFIG_AS_STRING, &str);
	c->error.colors[ERROR_COLOR_DEBUG_FILE] = al_color_name(str);

	if (config_get_value("font:error", "file", CONFIG_AS_STRING, &str))
		c->error.fonts[ERROR_FONT_DEFAULT].file = str;

	if (config_get_value("font:error", "size", CONFIG_AS_INT, &inum))
		c->error.fonts[ERROR_FONT_DEFAULT].size = inum;

	for (int i = 0; i < ERROR_FONT_MAX; i++)
		c->error.fonts[i].flags = 0;

	return true;
}


bool
config_save(CONFIG *c)
{
#ifndef _DEBUG
	static ALLEGRO_PATH *p;
#endif
	static const char *file;
	static bool retval;
	ALLEGRO_CONFIG *merged;

	assert(c != NULL);
	assert(current != NULL);

#ifndef _DEBUG
	p = al_get_standard_path(ALLEGRO_RESOURCES_PATH);
	al_set_path_filename(p, USER_CONFIG_FILE);
	file = al_path_cstr(p, ALLEGRO_NATIVE_PATH_SEP);
#else
	file = USER_CONFIG_FILE;
#endif

	/* Values in configuration 'cfg2' override those in 'cfg1'. */
	merged = al_merge_config(defaults, current);
	retval = al_save_config_file(file, merged);
#ifndef _DEBUG
	al_destroy_path(p);
#endif
	return retval;
}


void
config_update(const char *section, const char *k, const char *v)
{
	assert(k != NULL);
	assert(current != NULL);
	al_set_config_value(current, section, k, v);
}

