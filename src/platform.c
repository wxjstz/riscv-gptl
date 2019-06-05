#include <platform.h>

extern struct platform_interface platform;

void platform_init(void)
{
    platform.init();
}

int platform_hart_num(void)
{
    return platform.hart_num;
}

size_t platform_logic_block_size(void)
{
    return platform.logic_block_size;
}

void *platform_read(size_t offset, size_t size, void *buff)
{
    return platform.read(offset, size, buff);
}

int putchar(int c)
{
    if (c == '\n')
        platform.putchar('\r');
    platform.putchar(c);
    return c;
}

int puts(const char *s)
{
    int r = 0;
    while (*s) {
        putchar(*s);
        s++;
        r++;
    }
    putchar('\n');
    r++;
    return r;
}
