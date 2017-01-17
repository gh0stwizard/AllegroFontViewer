#ifndef _AFV_MAIN_H__
#define _AFV_MAIN_H__

#include "engine.h"
#include "utils.h"
#include "config.h"

ALLEGRO_TRANSFORM		*camera;
ALLEGRO_DISPLAY			*display;
ALLEGRO_EVENT_QUEUE		*event_queue;
ALLEGRO_MONITOR_INFO	*monitor;

CONFIG *CFG;

enum {
	TIMER_MAIN,
	TIMER_MAX
};
ALLEGRO_TIMER			*timers[TIMER_MAX];

enum {
	BITMAP_ICON,
	BITMAP_MAX
};
ALLEGRO_BITMAP			*bitmaps[BITMAP_MAX];

ALLEGRO_FONT			*fonts[FONT_MAX];

void init(void);
void loop(void);
void tini(void);

#endif /* _AFV_MAIN_H__ */
