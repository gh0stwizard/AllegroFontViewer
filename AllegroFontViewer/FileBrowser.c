#include "FileBrowser.h"
#include "vector.h"
#include "colors.h"

#include <assert.h>
#if defined(ALLEGRO_WINDOWS)
#include <string.h>
#else
#include <strings.h>
#endif


struct FILEBROWSER {
	ALLEGRO_BITMAP *b;
	int w, h;
	FONT fonts[FILEBROWSER_FONT_MAX];
	ALLEGRO_COLOR colors[FILEBROWSER_COLOR_MAX];
	VECTOR *d; /* An array of ALLEGRO_PATH* for directories */
	VECTOR *f; /* An array of ALLEGRO_PATH* for files */
	size_t selected; /* index of the selected _directory_ element */
	size_t eldrawed; /* amount of elements drawed last time */
	size_t startpos;
	int px, py; /* padding for an element */
	ALLEGRO_PATH *curdir;
	bool changedir;
	struct { ALLEGRO_USTR *dir, *file; } prefix;
	struct {
		void(*sort_dirs)(FILEBROWSER *fb);
		void(*sort_file)(FILEBROWSER *fb);
	} hook;
	struct {
		size_t selected;
		size_t startpos;
		ALLEGRO_PATH *p;
	} previous;
	struct {
		size_t directories;
		size_t files;
	} counter;
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

	fb->colors[FILEBROWSER_COLOR_FONT_TTF_BG] = COLOR_NORMAL_BLACK;
	fb->colors[FILEBROWSER_COLOR_FONT_TTF_FG] = COLOR_NORMAL_BLUE;

	fb->colors[FILEBROWSER_COLOR_FONT_OTF_BG] = COLOR_NORMAL_BLACK;
	fb->colors[FILEBROWSER_COLOR_FONT_OTF_FG] = COLOR_BRIGHT_BLUE;


	fb->px = 5;
	fb->py = 2;

	fb->changedir = true;

	fb->prefix.dir = al_ustr_new("/");
	fb->prefix.file = al_ustr_new(" ");

	fb->eldrawed = 0;

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

	if (fb->previous.p != NULL)
		al_destroy_path(fb->previous.p);
	fb->previous.p = NULL;

	fb->b = NULL;
	fb->f = NULL;
	fb->d = NULL;

	al_free(fb);
}


bool
filebrowser_load_fonts(FILEBROWSER *fb, FONT fontlist[])
{
	static FONT fi;
	static ALLEGRO_FONT *font;

	assert(fb != NULL);
	for (int i = 0; i < FILEBROWSER_FONT_MAX; i++) {
		fi = fontlist[i];
		font = al_load_font(fi.file, fi.size, fi.flags);
		assert(font != NULL);

		fb->fonts[i] = fi;

		if (fb->fonts[i].font != NULL) {
			al_destroy_font(fb->fonts[i].font);
			fb->fonts[i].font = NULL;
		}

		fb->fonts[i].font = font;
		fb->fonts[i].height = al_get_font_line_height(font);
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
	static const char *ename;
	static bool retval;
	extern int fs_entry_cb(ALLEGRO_FS_ENTRY *, void *);
	static size_t selected, startpos; /* temp. buffer for new values */

	assert(path != NULL);
	entry = al_create_fs_entry(path);
	retval = false;

	if (!al_fs_entry_exists(entry))
		goto done;

	if (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) {
		ename = al_get_fs_entry_name(entry);

		if (fb->changedir && !al_change_directory(ename))
			goto done;

		/* Compare the old directory with current one and
		 * if we were here before then restore the position.
		 */
		selected = 0;
		startpos = 0;
		if (fb->previous.p != NULL) {
			/* TODO:
			 * 1. Use a brand new al_path_ustr()
			 * 2. Is ICU fits here better than al_ustr_compare?
			 */
			ALLEGRO_PATH *p = al_create_path_for_directory(ename);
			ALLEGRO_USTR *old = al_ustr_new(al_path_cstr(fb->previous.p,
				ALLEGRO_NATIVE_PATH_SEP));
			ALLEGRO_USTR *cur = al_ustr_new(al_path_cstr(p,
				ALLEGRO_NATIVE_PATH_SEP));
			if (al_ustr_compare(cur, old) == 0) {
				selected = fb->previous.selected;
				startpos = fb->previous.startpos;
			}
			al_destroy_path(p);
			al_ustr_free(old);
			al_ustr_free(cur);
		}

		/* remember old directory & position */
		if (fb->curdir != NULL) {
			fb->previous.selected = fb->selected;
			fb->previous.startpos = fb->startpos;
			if (fb->previous.p != NULL)
				al_destroy_path(fb->previous.p);
			fb->previous.p = fb->curdir;
		}

		/* restore/reset current values */
		fb->selected = selected;
		fb->startpos = startpos;
		fb->eldrawed = 0;
		/* remember current directory path */
		fb->curdir = al_create_path_for_directory(ename);

		/* reset counters */
		fb->counter.directories = 0;
		fb->counter.files = 0;

		if (al_open_directory(entry)) {
			switch (al_for_each_fs_entry(entry, fs_entry_cb, fb)) {
			case ALLEGRO_FOR_EACH_FS_ENTRY_OK:
				retval = true;
				fb->counter.directories = vector_count(fb->d);
				fb->counter.files = vector_count(fb->f);
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

done:
	al_destroy_fs_entry(entry);
	return retval;
}


bool
filebrowser_browse_selected(FILEBROWSER *fb)
{
	static ALLEGRO_PATH *p;
	static size_t selected;

	assert(fb != NULL);
	selected = fb->selected;

	if (selected < fb->counter.directories) {
		assert(vector_get(fb->d, selected, (void **)&p));
		return filebrowser_change_path(fb, p);
	}
	else
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
			return filebrowser_change_path(fb, p);
		}
	}
	else {
		p = al_create_path_for_directory("..");
		return filebrowser_change_path(fb, p);
	}

	return false;
}


static void
filebrowser_reset_vector(VECTOR *V)
{
	static size_t i, count;
	static ALLEGRO_PATH *p;

	for (i = 0, count = vector_count(V); i < count; i++) {
		assert(vector_get(V, i, (void **)&p));
		al_destroy_path(p);
		p = NULL;
	}
	assert(vector_reset(V));
}


bool
filebrowser_change_path(FILEBROWSER *fb, ALLEGRO_PATH *np)
{
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
filebrowser_select_prev_items(FILEBROWSER *fb, int percent)
{
	static size_t count, selected, op;

	assert(fb != NULL);

	count = vector_count(fb->d) + vector_count(fb->f);
	selected = fb->selected;

	if (count == 0)
		return false;

	if (percent > 100)
		percent = 100;

	op = (fb->eldrawed * percent) / 100;

	if (op <= 0)
		op = 1;

	if (selected >= op)
		fb->selected -= op;
	else
		fb->selected = 0;

	return true;
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


bool
filebrowser_select_next_items(FILEBROWSER *fb, int percent)
{
	static size_t count, selected, op;

	assert(fb != NULL);

	count = vector_count(fb->d) + vector_count(fb->f);
	selected = fb->selected;

	if (count == 0)
		return false;

	if (percent > 100)
		percent = 100;

	op = (fb->eldrawed * percent) / 100;

	if (op <= 0)
		op = 1;

	if ((selected + op) >= count)
		fb->selected = count - 1;
	else
		fb->selected += op;

	return true;
}


#if defined(ALLEGRO_WINDOWS)
#define CMP(s1, s2, count) (_strnicmp((s1), (s2), (count)))
#else
#define CMP(s1, s2, count) (strncasecmp((s1), (s2), (count)))
#endif

void
filebrowser_draw(FILEBROWSER *fb)
{
	static ALLEGRO_FONT *F;
	static VECTOR *V;
	static size_t i, k, count, selected, pos, maxel, drawed;
	static const char *cstr;
	static ALLEGRO_USTR *ustr;
	static size_t ustr_offset;
	static ALLEGRO_PATH *p;
	static int w, h, maxH; /* max height or bottom */

	static int fx, fy; /* font coord. */
	static int fpx, fpy; /* font padding */
	static int fsize;

	static int x1, x2, y1, y2; /* element coord. */
	static int epx, epy; /* element padding */
	static int eH; /* total height of an element */

	static ALLEGRO_COLOR bg, fg;	/* colors for an el. (default) */
	static ALLEGRO_COLOR sbg, sfg;	/* same as above for the selected el. */
	static ALLEGRO_COLOR ttf_bg, ttf_fg;
	static ALLEGRO_COLOR otf_bg, otf_fg;
	static ALLEGRO_COLOR ttc_bg, ttc_fg;

	assert(fb != NULL);
	F = fb->fonts[FILEBROWSER_FONT_DEFAULT].font;
	V = fb->d;
	w = fb->w;
	h = fb->h;

	fpx = fb->fonts[FILEBROWSER_FONT_DEFAULT].px;
	fpy = fb->fonts[FILEBROWSER_FONT_DEFAULT].py;
	epx = fb->px;
	epy = fb->py;
	fsize = fb->fonts[FILEBROWSER_FONT_DEFAULT].size;
	eH = fsize + (2 * fpy) + 1;
	maxH = h - eH;

	bg = fb->colors[FILEBROWSER_COLOR_BACKGROUND];
	fg = fb->colors[FILEBROWSER_COLOR_FOREGROUND];

	/* start drawing */
	al_set_target_bitmap(fb->b);
	al_clear_to_color(bg);

	/* heading */
	{
		ALLEGRO_COLOR border_color = fb->colors[FILEBROWSER_COLOR_BORDER];
		float lh = fsize / 2 + epy;
		int bpx = 2;
		al_draw_line(bpx, h, w - bpx, h, border_color, 1); /* bottom hor. */
		al_draw_line(bpx, lh, bpx, h, border_color, 1);
		al_draw_line(w - bpx, lh, w - bpx, h, border_color, 1);
		al_draw_line(1, lh, epx - 1, lh, border_color, 1);

		assert(fb->curdir != NULL);
		cstr = al_path_cstr(fb->curdir, ALLEGRO_NATIVE_PATH_SEP);
		int headlen = al_get_text_width(F, cstr) + epx;
		al_draw_line(headlen, lh, w - bpx, lh, border_color, 1);

		al_draw_text(F, fg, epx, epy, 0, cstr);
	}

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

	ttf_bg = fb->colors[FILEBROWSER_COLOR_FONT_TTF_BG];
	ttf_fg = fb->colors[FILEBROWSER_COLOR_FONT_TTF_FG];
	otf_bg = fb->colors[FILEBROWSER_COLOR_FONT_OTF_BG];
	otf_fg = fb->colors[FILEBROWSER_COLOR_FONT_OTF_FG];
	ttc_bg = fb->colors[FILEBROWSER_COLOR_FONT_TTC_BG];
	ttc_fg = fb->colors[FILEBROWSER_COLOR_FONT_TTC_FG];

	/* in case if something changed in the previous directory */
	if (fb->selected >= (vector_count(fb->d) + vector_count(fb->f)))
		fb->selected = fb->startpos = 0;

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
	drawed = 0;
	for (i = pos, count = vector_count(V); i < count && y1 < maxH; i++) {
		assert(vector_get(V, i, (void **)&p));
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
		drawed++;
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
		assert(vector_get(V, k, (void **)&p));
		cstr = al_get_path_filename(p);
		al_ustr_truncate(ustr, (int)ustr_offset);
		al_ustr_append_cstr(ustr, cstr);
		if (selected == k) {
			al_draw_filled_rectangle(x1, y1, x2, y2, sbg);
			al_draw_ustr(F, sfg, fx, fy, 0, ustr);
		}
		else {
			const char *ext = al_get_path_extension(p);
			if (CMP(ext, ".ttf", 5) == 0) {
				al_draw_filled_rectangle(x1, y1, x2, y2, ttf_bg);
				al_draw_ustr(F, ttf_fg, fx, fy, 0, ustr);
			}
			else if (CMP(ext, ".otf", 5) == 0) {
				al_draw_filled_rectangle(x1, y1, x2, y2, otf_bg);
				al_draw_ustr(F, otf_fg, fx, fy, 0, ustr);
			}
			else if (CMP(ext, ".ttc", 5) == 0) {
				al_draw_filled_rectangle(x1, y1, x2, y2, ttc_bg);
				al_draw_ustr(F, ttc_fg, fx, fy, 0, ustr);
			}
			else {
				al_draw_filled_rectangle(x1, y1, x2, y2, bg);
				al_draw_ustr(F, fg, fx, fy, 0, ustr);
			}
		}
		y1 = y2 + epy;
		y2 = y1 + eH;
		fy = y1 + fpy;
		drawed++;
	}
	al_ustr_free(ustr);
	fb->eldrawed = drawed;
}


ALLEGRO_BITMAP *
filebrowser_bitmap(FILEBROWSER *fb)
{
	assert(fb != NULL);
	return fb->b;
}


void
filebrowser_set_hook(FILEBROWSER *fb, uint8_t id, void(*hook)(FILEBROWSER *))
{
	assert(fb != NULL);
	switch (id) {
	case FILEBROWSER_HOOK_SORT_DIRS:
		assert(hook != NULL);
		fb->hook.sort_dirs = hook;
		break;
	case FILEBROWSER_HOOK_SORT_FILE:
		assert(hook != NULL);
		fb->hook.sort_file = hook;
		break;
	}
}


void
filebrowser_sort(FILEBROWSER *fb, uint8_t type,
	int(*cmp)(const void *, const void *))
{
	if (type)
		vector_qsort(fb->f, cmp);
	else
		vector_qsort(fb->d, cmp);
}


ALLEGRO_PATH *
filebrowser_get_selected_path(FILEBROWSER *fb)
{
	static ALLEGRO_PATH *p;
	static size_t selected, num_dirs;

	assert(fb != NULL);
	selected = fb->selected;
	num_dirs = fb->counter.directories;

	if ((num_dirs == 0) && (fb->counter.files == 0))
		return NULL;

	if (selected >= num_dirs) {
		selected -= num_dirs;
		assert(vector_get(fb->f, selected, (void **)&p));
		return al_clone_path(p);
	}
	else
		return NULL;
}


void
filebrowser_set_colors(FILEBROWSER *fb, ALLEGRO_COLOR list[])
{
	assert(fb != NULL);

	for (int i = 0; i < FILEBROWSER_COLOR_MAX; i++) {
		fb->colors[i] = list[i];
	}
}


ALLEGRO_PATH *
filebrowser_get_current_path(FILEBROWSER *fb)
{
	assert(fb != NULL);
	return fb->curdir;
}


void
filebrowser_resize(FILEBROWSER *self, int w, int h)
{
	assert(self != NULL);


	self->w = w;
	self->h = h;

	if (self->b != NULL) {
		al_destroy_bitmap(self->b);
		self->b = NULL;
	}

	self->b = al_create_bitmap(self->w, self->h);
	assert(self->b);
}
