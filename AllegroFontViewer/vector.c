/* Originally written by Emil Hernvall
 *     https://gist.github.com/953968
 * Peter Hornyack and Katelin Bailey, 12/8/11, University of Washington
 *     https://gist.github.com/pjh/1453219
 */
#include "vector.h"

#include <stdlib.h>
#include <errno.h>

#if defined(_WIN32) || defined(_WIN64)
#include <search.h>	/* qsort_s */
#endif

#define VECTOR_INIT_SIZE		2
#define VECTOR_RESIZE_FACTOR	2
#define VECTOR_MAX ((UINT_MAX / (sizeof(void *) * VECTOR_RESIZE_FACTOR)) - 1)

#ifdef __cplusplus
extern "C" {
#endif

struct _afv_vector {
	void **data;
	size_t count;
	size_t size;
};

/* TODO: thread-local storage
 */
static int vector_errnum;

void
vector_set_errno(int errnum)
{
	vector_errnum = errnum;
}

int
vector_get_errno(void)
{
	return vector_errnum;
}


VECTOR *
vector_new(void)
{
	VECTOR *l = NULL;

	if ((l = malloc(sizeof(VECTOR))) != NULL) {
		l->count = 0;
		l->size = VECTOR_INIT_SIZE;
		l->data = malloc(sizeof(void *) * l->size);
		if (l->data == NULL) {
			vector_set_errno(errno);
			free(l);
			l = NULL;
		}
	}
	else
		vector_set_errno(errno);

	return l;
}

void
vector_destroy(VECTOR *l)
{
	if (l == NULL)
		return;

	if (l->data != NULL)
		free(l->data);

	free(l);
}

bool
vector_add(VECTOR *l, void *e)
{
	static void **temp;

	if (l == NULL) {
		vector_set_errno(EINVAL);
		return false;
	}

	/* if the current allocated size of vector struct. is exceeding the value
	 * below, we throw an error.
	 */
	if (l->size >= VECTOR_MAX) {
		vector_set_errno(ERANGE);
		return false;
	}

	if (l->size == l->count) {
		l->size *= VECTOR_RESIZE_FACTOR;
		temp = realloc(l->data, sizeof(void *) * l->size);
		if (temp != NULL) {
			l->data = temp;
		}
		else {
			vector_set_errno(errno);
			return false;
		}
	}

	l->data[l->count] = e;
	l->count++;

	return true;
}

bool
vector_get(VECTOR *l, size_t index, void **e)
{
	if ((l == NULL) || (e == NULL)) {
		vector_set_errno(EINVAL);
		return false;
	}

	if (index >= l->count) {
		vector_set_errno(ERANGE);
		return false;
	}

	*e = l->data[index];
	return true;
}

bool
vector_shrink(VECTOR *l)
{
	static void **temp;

	if ((l->size > VECTOR_INIT_SIZE)
		&& (l->count <= l->size / (VECTOR_RESIZE_FACTOR * 2)))
	{
		l->size /= VECTOR_RESIZE_FACTOR;
		temp = realloc(l->data, sizeof(void *) * l->size);
		if (temp != NULL) {
			l->data = temp;
		}
		else {
			vector_set_errno(errno);
			return false;
		}
	}

	return true;
}

bool
vector_delete(VECTOR *l, size_t index, void **e)
{
	static size_t i, t;

	if (l == NULL) {
		vector_set_errno(EINVAL);
		return false;
	}

	if (index >= l->count) {
		vector_set_errno(ERANGE);
		return false;
	}

	if (e != NULL)
		*e = l->data[index];

	for (i = index, t = l->count - 1; i < t; i++)
		l->data[i] = l->data[i + 1];
	l->count--;

	return vector_shrink(l);
}

void
vector_free_contents(VECTOR *l)
{
	size_t i, count;

	for (i = 0, count = l->count; i < count; i++)
		if (l->data[i] != NULL)
			free(l->data[i]);
}

bool
vector_reset(VECTOR *l)
{
	void **temp = realloc(l->data, sizeof(void *) * VECTOR_INIT_SIZE);
	if (temp != NULL) {
		l->data = temp;
		l->count = 0;
		l->size = VECTOR_INIT_SIZE;
		return true;
	}
	else {
		vector_set_errno(errno);
		return false;
	}
}

bool
vector_set(VECTOR *l, size_t index, void *e, void **old)
{
	if (l == NULL) {
		vector_set_errno(EINVAL);
		return false;
	}

	if (index >= l->count) {
		vector_set_errno(ERANGE);
		return false;
	}

	if (old != NULL)
		*old = l->data[index];

	l->data[index] = e;

	return true;
}

size_t
vector_count(VECTOR *l)
{
	if (l != NULL)
		return l->count;
	else
		return -1;
}

void
vector_qsort(VECTOR *l, int(*cmp)(const void *, const void *))
{
	qsort((void *)l->data, l->count, sizeof(void *), cmp);
}

#if defined(_WIN32) || defined(_WIN64)
void
vector_qsort_s(VECTOR *l,
	int(*cmp)(void *, const void *, const void *), void * ctx)
{
	qsort_s((void *)l->data, l->count, sizeof(void *), cmp, ctx);
}
#endif

#if defined(_GNU_SOURCE)
void
vector_qsort_r(VECTOR *l,
	int(*cmp)(const void *, const void *, void *), void * ctx)
{
	qsort_r((void *)l->data, l->count, sizeof(void *), cmp, ctx);
}
#endif

#ifdef __cplusplus
}
#endif
