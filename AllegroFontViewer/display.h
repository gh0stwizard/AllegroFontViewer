#ifndef _AFV_DISPLAY_H__
#define _AFV_DISPLAY_H__

#include "engine.h"


typedef struct DISPLAY_INFO {
	bool fullscreen;
	bool fswindowed;
	int w;
	int h;
	bool vsync;
} DISPLAY_INFO;


ALLEGRO_DISPLAY * create_display(DISPLAY_INFO *di);


#endif /* _AFV_DISPLAY_H__ */

