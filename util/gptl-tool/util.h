#ifndef __UTIL_H__
#define __UTIL_H__

size_t file_size(const char *path);

void save_file(void *buff, size_t size, const char *path);

void *load_file(void *buff, const char *path);

#endif
