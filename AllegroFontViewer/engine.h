#ifndef _AFV_ENGINE_H__
#define _AFV_ENGINE_H__

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


typedef struct FONT {
	ALLEGRO_FONT *font;
	char *file;
	int size;
	int height;
	int flags;
	int px;
	int py;
} FONT;


#endif /* _AFV_ENGINE_H__ */
