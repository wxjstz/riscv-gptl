#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <stdint.h>

struct platform_interface {
	int hart_num;
	char *platform_name;
	void (*init) (void);
	void (*putchar) (char c);
	size_t logic_block_size;
	void *(*read) (size_t offset, size_t size, void *buff);
};

int putchar(int c);

int puts(const char *s);

void platform_init(void);

int platform_hart_num(void);

void *platform_read(size_t offset, size_t size, void *buff);

size_t platform_logic_block_size(void);

#endif				/* __PLATFORM_H__ */
