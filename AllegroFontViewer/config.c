#include "config.h"

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <stdio.h>

#define DEFAULT_CONFIG_FILE	"defaults.ini"
#define USER_CONFIG_FILE	"settings.ini"

static ALLEGRO_CONFIG *defaults, *current;

CONFIG *
config_new(const char * const file)
{
	static ALLEGRO_PATH *p;

	CONFIG *cfg = calloc(1, sizeof(CONFIG));
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

	al_destroy_config(defaults);
	al_destroy_config(current);
	defaults = NULL;
	current = NULL;

	return cfg;
}

void
config_destroy(CONFIG *c)
{
	if (c == NULL)
		return;

	if (c->window.title)
		free(c->window.title);

	if (c->window.icon)
		free(c->window.icon);

	for (int i = 0; i < FONT_MAX; i++) {
		if (c->fonts[i].file != NULL) {
			free(c->fonts[i].file);
			c->fonts[i].file = NULL;
		}
	}

	for (int i = 0; i < FILEBROWSER_FONT_MAX; i++) {
		if (c->browser.fonts[i].file != NULL) {
			free(c->browser.fonts[i].file);
			c->browser.fonts[i].file = NULL;
		}
	}

	if (c->browser.startpath != NULL)
		free(c->browser.startpath);

	free(c);
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
#define STRNCPY(a, b, c, d) (do {\
len = strlen(c);\
strncpy(a, c, ((len >= d) ? d : len));\
} while (0))
#endif

#define STR_BUFSIZE 1023

const char *
config_get_value(const char *section, const char *key, int type, void *out)
{
	static int i;
	static double d;
	static size_t len, size;
	static char *dst;
	const char *src = _get_value(section, key);

	if (src == NULL)
		src = "";

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
		dst = (char *)malloc(size);
		assert(dst != NULL);
		len = strlen(src);
		STRNCPY(dst, size, src, STR_BUFSIZE);
		*(char **)out = dst;
		break;
	}

	return src;
}

bool
parse_config(CONFIG *c)
{
	int inum;
	float flt;
	char *str = NULL;

	/* FIXME: auto generate this code or auto-gen structure! */

	/* window */
	config_get_value("window", "title", CONFIG_AS_STRING, &str);
	c->window.title = str;

	config_get_value("window", "icon", CONFIG_AS_STRING, &str);
	c->window.icon = str;

	/* fonts */
	config_get_value("font:browser", "file", CONFIG_AS_STRING, &str);
	c->fonts[FONT_BROWSER].file = str;
	config_get_value("font:browser", "size", CONFIG_AS_INT, &inum);
	c->fonts[FONT_BROWSER].size = inum;

	config_get_value("font:status", "file", CONFIG_AS_STRING, &str);
	c->fonts[FONT_STATUS].file = str;
	config_get_value("font:status", "size", CONFIG_AS_INT, &inum);
	c->fonts[FONT_STATUS].size = inum;

	for (int i = 0; i < FONT_MAX; i++) { /* TODO */
		c->fonts[i].flags = 0;
	}

	/* display */
	config_get_value("display", "width", CONFIG_AS_INT, &inum);
	c->display.w = inum;

	config_get_value("display", "height", CONFIG_AS_INT, &inum);
	c->display.h = inum;

	config_get_value("display", "rate", CONFIG_AS_FLOAT, &flt);
	c->display.rate = flt;

	config_get_value("display", "vsync", CONFIG_AS_INT, &inum);
	c->display.vsync = !(inum == 0);

	/* colors */
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

	config_get_value("colors:status", "background", CONFIG_AS_STRING, &str);
	c->status.colors[STATUS_COLOR_BACKGROUND] = al_color_name(str);
	config_get_value("colors:status", "foreground", CONFIG_AS_STRING, &str);
	c->status.colors[STATUS_COLOR_FOREGROUND] = al_color_name(str);
	config_get_value("colors:status", "border", CONFIG_AS_STRING, &str);
	c->status.colors[STATUS_COLOR_BORDER] = al_color_name(str);

	/* keyboard */
	config_get_value("keyboard", "repeatrate", CONFIG_AS_INT, &inum);
	c->keyboard.repeatrate = inum;

	/*browser*/
	config_get_value("browser", "startpath", CONFIG_AS_STRING, &str);
	c->browser.startpath = str;
	config_get_value("browser", "scrollspeed", CONFIG_AS_INT, &inum);
	c->browser.scrollspeed = inum;

	/* viewer */
	for (int i = 0; i < VIEWER_MAX_PRESETS; i++) {
		char preset_str[8] = { 'p', 'r', 'e', 's', 'e', 't', i + 48, '\0' };
		config_get_value("viewer", preset_str, CONFIG_AS_STRING, &str);
		c->viewer.presets[i] = al_ustr_new((str == NULL) ? ("") : (str));
	}

	config_get_value("viewer", "minsize", CONFIG_AS_INT, &inum);
	c->viewer.minsize = inum;
	config_get_value("viewer", "maxsize", CONFIG_AS_INT, &inum);
	c->viewer.maxsize = inum;

	config_get_value("colors:viewer", "background", CONFIG_AS_STRING, &str);
	c->viewer.colors[FONTVIEWER_COLOR_BACKGROUND] = al_color_name(str);
	config_get_value("colors:viewer", "foreground", CONFIG_AS_STRING, &str);
	c->viewer.colors[FONTVIEWER_COLOR_FOREGROUND] = al_color_name(str);

	/* typer */
	config_get_value("font:typer", "file", CONFIG_AS_STRING, &str);
	c->typer.fonts[TYPER_FONT_DEFAULT].file = str;
	config_get_value("font:typer", "size", CONFIG_AS_INT, &inum);
	c->typer.fonts[TYPER_FONT_DEFAULT].size = inum;

	for (int i = 0; i < TYPER_FONT_MAX; i++)
		c->typer.fonts[i].flags = 0;

	/* help */
	config_get_value("colors:help", "background", CONFIG_AS_STRING, &str);
	c->help.colors[HELP_MENU_COLOR_BACKGROUND] = al_color_name(str);
	config_get_value("colors:help", "foreground", CONFIG_AS_STRING, &str);
	c->help.colors[HELP_MENU_COLOR_FOREGROUND] = al_color_name(str);

	config_get_value("font:help", "file", CONFIG_AS_STRING, &str);
	c->help.fonts[HELP_MENU_FONT_DEFAULT].file = str;
	config_get_value("font:help", "size", CONFIG_AS_INT, &inum);
	c->help.fonts[HELP_MENU_FONT_DEFAULT].size = inum;

	for (int i = 0; i < HELP_MENU_FONT_MAX; i++)
		c->help.fonts[i].flags = 0;

	return true;
}
