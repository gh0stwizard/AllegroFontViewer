#include "statusline.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

struct _afv_statusline {
	ALLEGRO_EVENT_SOURCE event_source;
	int type;
};

STATUSLINE *
statusline_new(int evid)
{
	STATUSLINE *sl = malloc(sizeof(STATUSLINE));

	assert(sl != NULL);
	al_init_user_event_source(&(sl->event_source));
	assert(evid > 1024);
	sl->type = evid;

	return sl;
}

void
statusline_destroy(STATUSLINE *sl)
{
	if (sl == NULL)
		return;

	free(sl);
}

static void
_dtor(ALLEGRO_USER_EVENT *ue)
{
	ALLEGRO_EVENT *e = (ALLEGRO_EVENT *)ue;
	al_ustr_free( (void *)e->user.data2 );
}

bool
statusline_push(STATUSLINE *sl, int subsystem, const char *msg)
{
	static ALLEGRO_EVENT ev;
	extern void _dtor(ALLEGRO_USER_EVENT *e);

	assert(sl != NULL);
	ev.user.type = sl->type;
	ev.user.data1 = subsystem;
	ev.user.data2 = (intptr_t)al_ustr_new(msg);

	return al_emit_user_event((ALLEGRO_EVENT_SOURCE *)sl, &ev, _dtor);
}
