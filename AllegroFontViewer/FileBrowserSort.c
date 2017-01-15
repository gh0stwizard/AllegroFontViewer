#include "FileBrowserSort.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <assert.h>
#include <errno.h>
#include "unicode/uclean.h"
#include "unicode/putil.h"
#include "unicode/ustring.h"
#include "unicode/ustdio.h"
#include "unicode/ucnv.h"
#include "unicode/ucol.h"


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

#define CARP(str) (fprintf(stderr, "%s: %s at line %d", __func__, (str), __LINE__))

bool
fbsort_init(void)
{
	UErrorCode error = U_ZERO_ERROR;

	u_init(&error);
	if (U_FAILURE(error)) {
		CARP(u_errorName(error));
		return false;
	}

	error = U_ZERO_ERROR;
	coll = ucol_open("en-u-kn-true", &error);
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
