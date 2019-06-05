#include <stdint.h>
#include <string.h>
#include <console.h>

/**
 * alloc once without release
 */
void *_malloc(size_t size)
{
    void *r = NULL;
    extern char __heap_start;   /* come from link script */
    extern char __heap_end;     /* come from link script */
    static uintptr_t free_ptr = (uintptr_t) & __heap_start;
    free_ptr = ALIGN_UP(free_ptr, __SIZEOF_POINTER__);
    if ((uintptr_t) & __heap_end - free_ptr < size)
        return NULL;
    r = (void *) free_ptr;
    free_ptr += size;
    return r;
}

void free(void *ptr)
{
}
