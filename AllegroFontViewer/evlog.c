#include "evlog.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>


struct EVLOG {
	ALLEGRO_EVENT_SOURCE event_source;
	int type; /* event type */
};


EVLOG *
evlog_new(int evid)
{
	EVLOG *evlog = malloc(sizeof(EVLOG));

	assert(evlog != NULL);
	al_init_user_event_source(&(evlog->event_source));
	assert(evid > 1024);
	evlog->type = evid;

	return evlog;
}


void
evlog_destroy(EVLOG *evlog)
{
	if (evlog == NULL)
		return;

	al_destroy_user_event_source(&(evlog->event_source));
	free(evlog);
}


static void
_dtor(ALLEGRO_USER_EVENT *ue)
{
	ALLEGRO_EVENT *e = (ALLEGRO_EVENT *)ue;
	al_ustr_free( (void *)e->user.data2 );
}


bool
evlog_push(EVLOG *evlog, int subsystem, const ALLEGRO_USTR *msg)
{
	static ALLEGRO_EVENT ev;
	extern void _dtor(ALLEGRO_USER_EVENT *e);

	/* TODO: use al_ustr_vappendf() */

	assert(evlog != NULL);
	ev.user.type = evlog->type;
	ev.user.data1 = subsystem;
	ev.user.data2 = (intptr_t)al_ustr_dup(msg);

	return al_emit_user_event((ALLEGRO_EVENT_SOURCE *)evlog, &ev, _dtor);
}


bool
evlog_push_cstr(EVLOG *evlog, int subsystem, const char *msg)
{
	static ALLEGRO_EVENT ev;
	extern void _dtor(ALLEGRO_USER_EVENT *e);

	assert(evlog != NULL);
	ev.user.type = evlog->type;
	ev.user.data1 = subsystem;
	ev.user.data2 = (intptr_t)al_ustr_new(msg);

	return al_emit_user_event((ALLEGRO_EVENT_SOURCE *)evlog, &ev, _dtor);
}


bool
evlog_pushf(EVLOG *evlog, int subsystem, const char *fmt, ...)
{
	static ALLEGRO_EVENT ev;
	static ALLEGRO_USTR *us;
	extern void _dtor(ALLEGRO_USER_EVENT *e);

	assert(evlog != NULL);

	va_list args;
	va_start(args, fmt);

	us = al_ustr_new("");
	assert(us != NULL);
	al_ustr_vappendf(us, fmt, args);

	va_end(args);

	ev.user.type = evlog->type;
	ev.user.data1 = subsystem;
	ev.user.data2 = (intptr_t)us;

	return al_emit_user_event((ALLEGRO_EVENT_SOURCE *)evlog, &ev, _dtor);
}
