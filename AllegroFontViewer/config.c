#include "config.h"
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <stdio.h>

#define DEFAULT_CONFIG_FILE	"defaults.ini"
#define USER_CONFIG_FILE	"settings.ini"


static ALLEGRO_CONFIG *defaults, *current;

bool
config_init(const char * const file)
{
	defaults = al_load_config_file(DEFAULT_CONFIG_FILE);
	assert(defaults);

	if (file == NULL)
		current = al_load_config_file(USER_CONFIG_FILE);
	else
		current = al_load_config_file(file);

	assert(current);
	return true;
}

void
config_cleanup(void)
{
	al_destroy_config(defaults);
	al_destroy_config(current);
}

static const char *
_get_value(const char *section, const char *key)
{
	static const char *retval;

	retval = al_get_config_value(current, section, key);

	if (retval == NULL)
		retval = al_get_config_value(defaults, section, key);

	return retval;
}

#if defined(ALLEGRO_WINDOWS)
#define ATOI(x) (atoi(x))
#define ATOF(x) (atof(x))
#else
#define ATOI(x) (strtol(x, NULL, 10))
#define ATOF(x) (strtod(x, NULL))
#endif

//#define STR_BUFSIZE 1024

const char *
config_get_value(const char *section, const char *key, int type, void *out)
{
	static void *temp;
	static int i;
	//static long long i64;
	static double d;
	const char *src = _get_value(section, key);

	if (src == NULL)
		return NULL;

	if (out == NULL && type == 0)
		return src;

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
		*(float *)out = (float)d;
		break;

//	case CONFIG_AS_STRING:
//		temp = (char *)malloc(sizeof(char) * STR_BUFSIZE + 1);
//		assert(temp != NULL);
//#if defined(ALLEGRO_WINDOWS)
//		memcpy_s(temp, STR_BUFSIZE, src, strnlen_s(src, STR_BUFSIZE));
//#else
//		memcpy(temp, src, strnlen(src, STR_BUFSIZE));
//#endif
//		out = temp;
//		break;
	}

	return src;
}
