#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdarg.h>
#include <asm.h>
/**
 * This function used to print character to console
 */
extern int putchar(int c);

int printf(const char *format, ...);
int sprintf(char *buff, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vsprintf(char *buff, const char *format, va_list ap);

#ifdef DEBUG
# define debug_printf(format, args...)\
	do {\
		printf(format, ##args);\
	} while (0)
#else
# define debug_printf(format, args...)
#endif

#define warning_printf(format, args...) \
	do {\
		printf(format, ##args);\
	} while (0)

#define error_printf(format, args...) \
	do {\
		printf(format, ##args);\
		while (1) wfi();\
	} while (0)

#define abort()	\
	do {\
		error_printf("abort at %s:%d\n", __FILE__, __LINE__);\
	} while (0)

#define assert(exp)	\
	do {\
		if (!(exp)) {\
			error_printf("assert(%s) at %s:%d\n", #exp, __FILE__, __LINE__);\
		}\
	} while (0)

#endif
