#ifndef _AFV_EVLOG_H__
#define _AFV_EVLOG_H__

#include "engine.h"

typedef struct EVLOG EVLOG;

EVLOG *
evlog_new(int evid);

void
evlog_destroy(EVLOG *evlog);

bool
evlog_push(EVLOG *evlog, int subsystem, const ALLEGRO_USTR *msg);

bool
evlog_push_cstr(EVLOG *evlog, int subsystem, const char *msg);

bool
evlog_pushf(EVLOG *evlog, int subsystem, const char *fmt, ...);


#endif /* _AFV_EVLOG_H__ */
