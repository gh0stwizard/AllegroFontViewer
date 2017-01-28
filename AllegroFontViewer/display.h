#ifndef _AFV_DISPLAY_H__
#define _AFV_DISPLAY_H__

#include "engine.h"


typedef struct DISPLAY_INFO {
	bool fullscreen;
	bool fswindowed;
	int w;
	int h;
	bool vsync;
	bool maximized;
	int x;
	int y;
	int window_w;
	int window_h;
	float framerate;
} DISPLAY_INFO;


ALLEGRO_DISPLAY *
create_display(DISPLAY_INFO *di);

bool
get_max_resolution(int *max_w, int *max_h);


#endif /* _AFV_DISPLAY_H__ */

