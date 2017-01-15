#ifndef _AFV_MAIN_H__
#define _AFV_MAIN_H__

#include "engine.h"
#include "utils.h"

#define WINDOW_TITLE	"Allegro Font Viewer"
#define ICON_FILE		"icon.bmp"
#define SCREEN_W		640
#define SCREEN_H		480
#define FPS				60.0

//#define DEFAULT_FONT		"fonts/droid/DroidSansMono.ttf"
#define DEFAULT_FONT		"fonts/noto/mono/NotoMono-Regular.ttf"
#define DEFAULT_FONT_SIZE	14
#define DEFAULT_FONT_FLAG	0

ALLEGRO_TRANSFORM		*camera;
ALLEGRO_DISPLAY			*display;
ALLEGRO_EVENT_QUEUE		*event_queue;
ALLEGRO_MONITOR_INFO	*monitor;

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

enum {
	FONT_DEFAULT,
	FONT_MAX
};
ALLEGRO_FONT			*fonts[FONT_MAX];

void init(void);
void loop(void);
void tini(void);

#endif /* _AFV_MAIN_H__ */
