#ifndef _AFV_STATUSLINE_H__
#define _AFV_STATUSLINE_H__

#include "engine.h"
#include "events.h"

enum {
	STATUS_COLOR_BACKGROUND,
	STATUS_COLOR_FOREGROUND,
	STATUS_COLOR_BORDER,
	STATUS_COLOR_MAX
};

typedef struct _afv_statusline STATUSLINE;

STATUSLINE *
statusline_new(int evid);

void
statusline_destroy(STATUSLINE *sl);

const ALLEGRO_USTR *
statusline_ustr(STATUSLINE *sl);

const char *
statusline_cstr(STATUSLINE *sl);

bool
statusline_push(STATUSLINE *sl, int subsystem, const char *msg);


#endif /* _AFV_STATUSLINE_H__ */
