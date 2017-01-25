#ifndef _AFV_MAIN_H__
#define _AFV_MAIN_H__

#include "engine.h"
#include "config.h"
#include "events.h"


enum {
	TIMER_STATUS,
	TIMER_KEYBOARD,
	TIMER_BLINK,
	TIMER_MAX
};


enum {
	BITMAP_ICON,
	BITMAP_MAX
};


ALLEGRO_TRANSFORM		*camera;
ALLEGRO_DISPLAY			*display;
ALLEGRO_EVENT_QUEUE		*event_queue;

ALLEGRO_TIMER			*timers[TIMER_MAX];
ALLEGRO_BITMAP			*bitmaps[BITMAP_MAX];

CONFIG					*CFG;

void init(void);
void loop(void);
void tini(void);

#endif /* _AFV_MAIN_H__ */
