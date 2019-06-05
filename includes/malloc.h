#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <console.h>

void *_malloc(size_t size);
void free(void *ptr);

#define malloc(size)\
	({\
		void *r = _malloc(size);\
		if (r == NULL)\
			error_printf("Cannot allocate memory, at %s:%d\n", __FILE__, __LINE__);\
		if (size == 0)\
			warning_printf("allocate memory of size zero, at %s:%d\n", __FILE__, __LINE__);\
		r;\
	})

#endif
