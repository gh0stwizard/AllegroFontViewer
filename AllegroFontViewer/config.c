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
	CONFIG *cfg = calloc(1, sizeof(CONFIG));
	assert(cfg != NULL);

	if (file == NULL) {
		defaults = al_load_config_file(DEFAULT_CONFIG_FILE);
		assert(defaults != NULL);
		current = al_load_config_file(USER_CONFIG_FILE);
	}
	else
		current = al_load_config_file(file);
	assert(current != NULL);

	parse_config(cfg);

	/* FIXME */
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
	config_get_value("font:default", "file", CONFIG_AS_STRING, &str);
	c->fonts[FONT_DEFAULT].file = str;

	config_get_value("font:default", "size", CONFIG_AS_INT, &inum);
	c->fonts[FONT_DEFAULT].size = inum;

	c->fonts[FONT_DEFAULT].flags = 0;

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

	return true;
}
