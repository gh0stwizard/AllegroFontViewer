#include "engine.h"
#include "messagebox.h"
#include <string.h>

void
mbox_warn_al_error(const char *header)
{
#if defined(_WIN32) || defined(_WIN64)
#define MSGSIZE 1024
	static char msgbuf[MSGSIZE];
	strerror_s(msgbuf, MSGSIZE - 1, al_get_errno());
	al_show_native_message_box(NULL, "Warning!", header, msgbuf, 
		NULL, ALLEGRO_MESSAGEBOX_WARN);
#undef MSGSIZE
#else
	//TBA strerror_r/strerror_l
	al_show_native_message_box(NULL, "Warning!", header, 
		strerror(al_get_errno()), NULL, ALLEGRO_MESSAGEBOX_WARN);
#endif
}

void
mbox_note(const char *header, const char *message)
{
	al_show_native_message_box(NULL, "Notification", header, message, 
		NULL, 0);
}
