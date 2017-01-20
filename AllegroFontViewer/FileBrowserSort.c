#ifndef _NO_ICU
#include "FileBrowserSort.h"

#include <assert.h>
#include "unicode/uclean.h"
#include "unicode/putil.h"
#include "unicode/ustring.h"
#include "unicode/ustdio.h"
#include "unicode/ucnv.h"
#include "unicode/ucol.h"


#define COLLATOR_MODE "en-u-kn-true"

static UCollator *coll;

static int
cmp_dirs(const void *a, const void *b)
{
	static UErrorCode error;
	static int retval;

	error = U_ZERO_ERROR;
	retval = ucol_strcollUTF8(coll,
		al_get_path_component(*(ALLEGRO_PATH **)a, -1), -1,
		al_get_path_component(*(ALLEGRO_PATH **)b, -1), -1,
		&error);
	assert(U_SUCCESS(error));
	return retval;
}

static int
cmp_file(const void *a, const void *b)
{
	static UErrorCode error;
	static int retval;

	error = U_ZERO_ERROR;
	retval = ucol_strcollUTF8(coll,
		al_get_path_filename(*(ALLEGRO_PATH **)a), -1,
		al_get_path_filename(*(ALLEGRO_PATH **)b), -1,
		&error);
	assert(U_SUCCESS(error));
	return retval;
}

bool
fbsort_init(void)
{
	UErrorCode error = U_ZERO_ERROR;

	u_init(&error);
	assert(U_SUCCESS(error));

	error = U_ZERO_ERROR;
	coll = ucol_open(COLLATOR_MODE, &error);
	assert(U_SUCCESS(error));

	return true;
}

void
fbsort_cleanup(void)
{
	ucol_close(coll);
}

void
fbsort_dirs(FILEBROWSER *fb)
{
	assert(fb != NULL);
	filebrowser_sort(fb, 0, cmp_dirs);
}

void
fbsort_file(FILEBROWSER *fb)
{
	assert(fb != NULL);
	filebrowser_sort(fb, 1, cmp_file);
}
#endif /* _NO_ICU */