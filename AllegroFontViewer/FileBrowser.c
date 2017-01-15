#include "FileBrowser.h"
#include "vector.h"
#include "colors.h"

#include <stdlib.h>
#include <assert.h>

#include <stdio.h> /* debug */

typedef struct _afv_filebrowser_font FILEBROWSER_FONT;
struct _afv_filebrowser_font {
	ALLEGRO_FONT *font;
	int size;
	int flags;
	int px, py;
};

struct _afv_filebrowser {
	ALLEGRO_BITMAP *b;
	int w, h;
	FILEBROWSER_FONT fonts[FILEBROWSER_FONT_MAX];
	ALLEGRO_COLOR colors[FILEBROWSER_COLOR_MAX];
	VECTOR *d; /* An array of ALLEGRO_PATH* for directories */
	VECTOR *f; /* An array of ALLEGRO_PATH* for files */
	size_t selected; /* index of the selected _directory_ element */
	size_t startpos;
	int px, py; /* padding for an element */
	ALLEGRO_PATH *curdir;
	bool changedir;
	struct { ALLEGRO_USTR *dir, *file; } prefix;
	uint8_t drawmode;
	struct {
		ALLEGRO_USTR *name;
		off_t size;
	} fileinfo;
	struct {
		void(*sort_dirs)(FILEBROWSER *fb);
		void(*sort_file)(FILEBROWSER *fb);
	} hook;
};

FILEBROWSER *
filebrowser_new(int width, int height)
{
	FILEBROWSER *fb = NULL;
	fb = (FILEBROWSER *)calloc(1, sizeof(FILEBROWSER));
	assert(fb != NULL);

	fb->w = width;
	fb->h = height;
	fb->b = al_create_bitmap(fb->w, fb->h);
	assert(fb->b != NULL);

	fb->d = vector_new();
	assert(fb->d != NULL);
	fb->f = vector_new();
	assert(fb->f != NULL);

	fb->colors[FILEBROWSER_COLOR_BACKGROUND] = COLOR_NORMAL_BLACK;
	fb->colors[FILEBROWSER_COLOR_FOREGROUND] = COLOR_BRIGHT_WHITE;

	fb->colors[FILEBROWSER_COLOR_DIR_BG] = COLOR_NORMAL_RED;
	fb->colors[FILEBROWSER_COLOR_DIR_FG] = COLOR_BRIGHT_WHITE;

	fb->colors[FILEBROWSER_COLOR_FILE_BG] = COLOR_NORMAL_BLACK;
	fb->colors[FILEBROWSER_COLOR_FILE_FG] = COLOR_NORMAL_WHITE;

	fb->colors[FILEBROWSER_COLOR_SELECT_BG] = COLOR_BRIGHT_CYAN;
	fb->colors[FILEBROWSER_COLOR_SELECT_FG] = COLOR_NORMAL_BLACK;

	fb->px = 5;
	fb->py = 2;

	fb->changedir = true;

	fb->prefix.dir = al_ustr_new("/");
	fb->prefix.file = al_ustr_new(" ");

	fb->drawmode = FILEBROWSER_DRAW_DIRLIST;

	return fb;
}

void
filebrowser_destroy(FILEBROWSER * fb)
{
	if (fb == NULL)
		return;

	if (fb->curdir != NULL)
		al_destroy_path(fb->curdir);
	fb->curdir = NULL;

	if (fb->prefix.dir != NULL)
		al_ustr_free(fb->prefix.dir);
	fb->prefix.dir = NULL;

	if (fb->prefix.file != NULL)
		al_ustr_free(fb->prefix.file);
	fb->prefix.file = NULL;

	for (int i = 0; i < FILEBROWSER_FONT_MAX; i++) {
		if (fb->fonts[i].font != NULL)
			al_destroy_font(fb->fonts[i].font);
		fb->fonts[i].font = NULL;
	}

	if (fb->b)
		al_destroy_bitmap(fb->b);
	if (fb->d)
		vector_destroy(fb->d);
	if (fb->f)
		vector_destroy(fb->f);

	fb->b = NULL;
	fb->f = NULL;
	fb->d = NULL;

	free(fb);
}

bool
filebrowser_load_font(FILEBROWSER *fb, FILEBROWSER_FONT_ID fontid,
	char const *file, int size, int flags)
{
	static ALLEGRO_FONT *font;

	assert(fb != NULL);
	assert(fontid < FILEBROWSER_FONT_MAX);

	font = al_load_font(file, size, flags);
	assert(font != NULL);

	if (font != NULL) {
		fb->fonts[fontid].font = font;
		fb->fonts[fontid].size = size;
		fb->fonts[fontid].flags = flags;
		fb->fonts[fontid].px = 5;
		fb->fonts[fontid].py = 5;
	}

	return true;
}

bool
filebrowser_set_font_padding(FILEBROWSER *fb, FILEBROWSER_FONT_ID fontid,
	int px, int py)
{
	assert(fb != NULL);
	assert(fontid < FILEBROWSER_FONT_MAX);
	fb->fonts[fontid].px = px;
	fb->fonts[fontid].py = py;
	return true;
}

static int
fs_entry_cb(ALLEGRO_FS_ENTRY *e, void *extra)
{
	static FILEBROWSER *fb;
	static ALLEGRO_PATH *p;
	static uint32_t emode;
	static const char *ename;

	fb = (FILEBROWSER *)extra;
	emode = al_get_fs_entry_mode(e);
	ename = al_get_fs_entry_name(e);

	if (emode & ALLEGRO_FILEMODE_HIDDEN) {
		return ALLEGRO_FOR_EACH_FS_ENTRY_SKIP;
	}

	if (emode & ALLEGRO_FILEMODE_ISDIR) {
		p = al_create_path_for_directory(ename);
		assert(vector_add(fb->d, p));
	}
	else {
		p = al_create_path(ename);
		assert(vector_add(fb->f, p));
	}
	return ALLEGRO_FOR_EACH_FS_ENTRY_SKIP; /* no recursion */
}

bool
filebrowser_browse_path(FILEBROWSER *fb, const char *path)
{
	static ALLEGRO_FS_ENTRY *entry;
	static uint32_t emode;
	static bool retval;
	extern int fs_entry_cb(ALLEGRO_FS_ENTRY *, void *);

	assert(path != NULL);
	entry = al_create_fs_entry(path);
	retval = false;

	if (!al_fs_entry_exists(entry))
		goto done;

	if (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) {
		if (fb->changedir) {
			if (!(al_change_directory(path)))
				goto done;
		}

		/* always reset to first el. */
		fb->selected = 0;
		fb->startpos = 0;

		/* no need to keep previous path anymore */
		if (fb->curdir != NULL)
			al_destroy_path(fb->curdir);
		fb->curdir = al_create_path_for_directory(path);

		if (al_open_directory(entry)) {
			//fprintf(stderr, "dir %s\n", path);
			switch (al_for_each_fs_entry(entry, fs_entry_cb, fb)) {
			case ALLEGRO_FOR_EACH_FS_ENTRY_OK:
				retval = true;
				break;
			case ALLEGRO_FOR_EACH_FS_ENTRY_ERROR: /* with    al_set_errno() */
			case ALLEGRO_FOR_EACH_FS_ENTRY_STOP:  /* without al_set_errno() */
				break;
			}
			al_close_directory(entry);

			if (fb->hook.sort_dirs != NULL)
				fb->hook.sort_dirs(fb);

			if (fb->hook.sort_file != NULL)
				fb->hook.sort_file(fb);
		}
	}
	else {
		ALLEGRO_PATH *p = al_create_path(path);
		if (fb->fileinfo.name != NULL) {
			ALLEGRO_USTR *n = fb->fileinfo.name;
			al_ustr_truncate(n, 0);
			al_ustr_append_cstr(n, al_path_cstr(p, ALLEGRO_NATIVE_PATH_SEP));
		}
		else {
			fb->fileinfo.name =
				al_ustr_new(al_path_cstr(p, ALLEGRO_NATIVE_PATH_SEP));
		}
		fb->fileinfo.size = al_get_fs_entry_size(entry);
		al_destroy_path(p);
		retval = true;
	}

done:
	al_destroy_fs_entry(entry);
	return retval;
}

bool
filebrowser_browse_selected(FILEBROWSER *fb)
{
	static ALLEGRO_PATH *p;
	static size_t selected, count;

	assert(fb != NULL);
	selected = fb->selected;
	count = vector_count(fb->d);

	if (selected >= count) {
		selected -= count;
		count = vector_count(fb->f);
		if ((count > 0) && (selected < count)) {
			assert(vector_get(fb->f, selected, &p));
			const char *path = al_path_cstr(p, ALLEGRO_NATIVE_PATH_SEP);
			if (filebrowser_browse_path(fb, path)) {
				fb->drawmode = FILEBROWSER_DRAW_FILEINFO;
				return true;
			}
		}
	}
	else {
		assert(vector_get(fb->d, selected, &p));
		if (filebrowser_change_path(fb, p)) {
			fb->drawmode = FILEBROWSER_DRAW_DIRLIST;
			return true;
		}
	}

	return false;
}

bool
filebrowser_browse_parent(FILEBROWSER *fb)
{
	static ALLEGRO_PATH *p;

	assert(fb != NULL);
	p = fb->curdir;
	if (p != NULL) {
		if (al_get_path_num_components(p)) {
			al_remove_path_component(p, -1);
			if (filebrowser_change_path(fb, p)) {
				fb->drawmode = FILEBROWSER_DRAW_DIRLIST;
				return true;
			}
		}
	}
	return false;
}

static void
filebrowser_reset_vector(VECTOR *V)
{
	static size_t i, count;
	for (i = 0, count = vector_count(V); i < count; i++) {
		ALLEGRO_PATH *p;
		assert(vector_get(V, i, &p));
		al_destroy_path(p);
	}
	assert(vector_reset(V));
}

bool
filebrowser_change_path(FILEBROWSER *fb, ALLEGRO_PATH *np)
{
	static size_t i, count;
	static VECTOR *V;
	static bool retval;
	static ALLEGRO_USTR *ustr;
	static const char *cstr;

	assert(fb != NULL);
	assert(np != NULL);
	cstr = al_path_cstr(np, ALLEGRO_NATIVE_PATH_SEP);
	ustr = al_ustr_new(cstr);
	filebrowser_reset_vector(fb->d);
	filebrowser_reset_vector(fb->f);
	retval = false;
	retval = filebrowser_browse_path(fb, al_cstr(ustr));
	al_ustr_free(ustr);
	return retval;
}

bool
filebrowser_select_prev(FILEBROWSER *fb)
{
	assert(fb != NULL);

	if (fb->selected == 0)
		return false;
		
	if (vector_count(fb->d) > 0) {
		fb->selected--;
		return true;
	}
	else if (vector_count(fb->f) > 0) {
		fb->selected--;
		return true;
	}

	return false;
}

bool
filebrowser_select_next(FILEBROWSER *fb)
{
	static size_t count, selected;

	assert(fb != NULL);
	selected = fb->selected;
	count = vector_count(fb->d) + vector_count(fb->f);
	if (count > (selected + 1)) {
		fb->selected++;
		return true;
	}
	else
		return false;
}

void
filebrowser_draw(FILEBROWSER *fb)
{
	static ALLEGRO_PATH *p;
	static long int selected;

	assert(fb != NULL);
	selected = (long)fb->selected;
	switch (fb->drawmode) {
	case FILEBROWSER_DRAW_DIRLIST:
		filebrowser_draw_dirlist(fb);
		break;
	case FILEBROWSER_DRAW_FILEINFO:
		filebrowser_draw_fileinfo(fb);
		break;
	}
}

void
filebrowser_draw_dirlist(FILEBROWSER *fb)
{
	static ALLEGRO_FONT *F;
	static VECTOR *V;
	static size_t i, k, count, selected, pos, maxel;
	static const char *cstr;
	static ALLEGRO_USTR *ustr;
	static size_t ustr_offset;
	static ALLEGRO_PATH *p;
	static int maxH; /* max height or bottom */
	
	static int fx, fy; /* font coord. */
	static int fpx, fpy; /* font padding */
	static int fsize;

	static int x1, x2, y1, y2; /* element coord. */
	static int epx, epy; /* element padding */
	static int eH; /* total height of an element */

	static ALLEGRO_COLOR bg, fg;	/* colors for an el. */
	static ALLEGRO_COLOR sbg, sfg;	/* same as above for the selected el. */

	assert(fb != NULL);
	F = fb->fonts[FILEBROWSER_FONT_DEFAULT].font;
	V = fb->d;
	fpx = fb->fonts[FILEBROWSER_FONT_DEFAULT].px;
	fpy = fb->fonts[FILEBROWSER_FONT_DEFAULT].py;
	epx = fb->px;
	epy = fb->py;
	fsize = fb->fonts[FILEBROWSER_FONT_DEFAULT].size;
	eH = fsize + (2 * fpy) + 1;
	maxH = fb->h - eH;

	bg = fb->colors[FILEBROWSER_COLOR_BACKGROUND];
	fg = fb->colors[FILEBROWSER_COLOR_FOREGROUND];

	/* start drawing */
	al_set_target_bitmap(fb->b);
	al_clear_to_color(bg);

	/* heading */
	al_draw_text(F, fg, epx, epy, 0, 
		al_path_cstr(fb->curdir, ALLEGRO_NATIVE_PATH_SEP));

	/* set coords. for first el. */
	x1 = epx;
	y1 = fsize + (2 * epy);
	x2 = fb->w - epx;
	y2 = y1 + eH;
	/* set coords. for font */
	fx = x1 + fpx;
	fy = y1 + fpy;

	sbg = fb->colors[FILEBROWSER_COLOR_SELECT_BG];
	sfg = fb->colors[FILEBROWSER_COLOR_SELECT_FG];

	bg = fb->colors[FILEBROWSER_COLOR_DIR_BG];
	fg = fb->colors[FILEBROWSER_COLOR_DIR_FG];

	/* count starting position */
	pos = fb->startpos;
	selected = fb->selected;
	// start + (eH + epy) * x < maxH
	// x = maxH - start / (eH + epy)
	maxel = (maxH - y1) / (eH + epy); /* does not take into account 1st el. */
	if (pos > selected)
		pos = selected;
	else if (selected >= (maxel + 1))
		if (selected - pos > maxel)
			pos = selected - maxel;
	/* remember new position for re-drawing purposes */
	fb->startpos = pos;
	/* output directories with their own prefix :) */
	ustr = al_ustr_dup(fb->prefix.dir);
	ustr_offset = al_ustr_size(ustr);
	for (i = pos, count = vector_count(V); i < count && y1 < maxH; i++) {
		assert(vector_get(V, i, &p));
		cstr = al_get_path_component(p, -1);
		al_ustr_truncate(ustr, (int)ustr_offset);
		al_ustr_append_cstr(ustr, cstr);
		if (i == selected) {
			al_draw_filled_rectangle(x1, y1, x2, y2, sbg);
			al_draw_ustr(F, sfg, fx, fy, 0, ustr);
		}
		else {
			al_draw_filled_rectangle(x1, y1, x2, y2, bg);
			al_draw_ustr(F, fg, fx, fy, 0, ustr);
		}
		y1 = y2 + epy;
		y2 = y1 + eH;
		fy = y1 + fpy;
	}
	/* drawing files... */
	V = fb->f;
	bg = fb->colors[FILEBROWSER_COLOR_FILE_BG];
	fg = fb->colors[FILEBROWSER_COLOR_FILE_FG];
	/* output files with their own prefix too */
	al_ustr_truncate(ustr, 0);
	al_ustr_append(ustr, fb->prefix.file);
	ustr_offset = al_ustr_size(ustr);
	//printf("s: %d i: %d d: %d pos: %d\n", selected, i, count, pos);
	/* adjust position and selected file position according to dir. counter */
	selected -= count;
	for (k = i - count, count = vector_count(V); k < count && y1 < maxH; k++) {
		assert(vector_get(V, k, &p));
		cstr = al_get_path_filename(p);
		al_ustr_truncate(ustr, (int)ustr_offset);
		al_ustr_append_cstr(ustr, cstr);
		if (selected == k) {
			al_draw_filled_rectangle(x1, y1, x2, y2, sbg);
			al_draw_ustr(F, sfg, fx, fy, 0, ustr);
		}
		else {
			al_draw_filled_rectangle(x1, y1, x2, y2, bg);
			al_draw_ustr(F, fg, fx, fy, 0, ustr);
		}
		y1 = y2 + epy;
		y2 = y1 + eH;
		fy = y1 + fpy;
	}
	al_ustr_free(ustr);
}

ALLEGRO_BITMAP *
filebrowser_bitmap(FILEBROWSER *fb)
{
	return fb->b;
}

void
filebrowser_draw_fileinfo(FILEBROWSER *fb)
{
	static ALLEGRO_FONT *F;
	static ALLEGRO_COLOR bg, fg;
	static ALLEGRO_USTR *ustr;
	static off_t esize;
	static int epx, epy; /* element padding */
	static int fx, fy; /* font coord. */
	static int fh;

	assert(fb != NULL);
	F = fb->fonts[FILEBROWSER_FONT_DEFAULT].font;
	bg = fb->colors[FILEBROWSER_COLOR_BACKGROUND];
	fg = fb->colors[FILEBROWSER_COLOR_FOREGROUND];
	epx = fb->px;
	epy = fb->py;
	fx = epx;
	fy = epy;
	fh = fb->fonts[FILEBROWSER_FONT_DEFAULT].size;

	/* start drawing */
	al_set_target_bitmap(fb->b);
	al_clear_to_color(bg);

	ustr = al_ustr_new("File: ");
	al_ustr_append(ustr, fb->fileinfo.name);
	al_draw_ustr(F, fg, fx, fy, 0, ustr);

	al_ustr_truncate(ustr, 0);
	ustr = al_ustr_newf("Size: %d",fb->fileinfo.size);
	fy += fh + epy;
	al_draw_ustr(F, fg, fx, fy, 0, ustr);

	al_ustr_free(ustr);;
}

uint8_t
filebrowser_draw_mode(FILEBROWSER *fb, uint8_t mode)
{
	static uint8_t old;

	assert(fb != NULL);
	old = fb->drawmode;
	switch (mode) {
	case FILEBROWSER_DRAW_DIRLIST:
	case FILEBROWSER_DRAW_FILEINFO:
		fb->drawmode = mode;
		break;
	}
	return old;
}

void
filebrowser_set_hook(FILEBROWSER *fb, uint8_t id, void(*hook)(FILEBROWSER *))
{
	assert(fb != NULL);
	switch (id) {
	case FILEBROWSER_HOOK_DIRSORT:
		assert(hook != NULL);
		fb->hook.sort_dirs = hook;
		break;
	case FILEBROWSER_HOOK_FILESORT:
		assert(hook != NULL);
		fb->hook.sort_file = hook;
		break;
	}
}

void
filebrowser_sort(FILEBROWSER *fb, uint8_t type,
	int (*cmp)(const void *, const void *))
{
	if (type)
		vector_qsort(fb->f, cmp);
	else
		vector_qsort(fb->d, cmp);
}
