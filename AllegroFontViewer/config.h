#ifndef _AFV_CONFIG_H__
#define _AFV_CONFIG_H__

#include "engine.h"

enum {
	CONFIG_AS_IS = 0,
	CONFIG_AS_INT,
	CONFIG_AS_FLOAT
};

bool
config_init(const char * const file);

void
config_cleanup(void);

const char *
config_get_value(const char *section, const char *key, int type, void *out);


#endif /* _AFV_CONFIG_H__ */
