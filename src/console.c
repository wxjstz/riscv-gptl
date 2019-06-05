#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <console.h>

#define MAX(a,b)	((a) >= (b) ? (a) : (b))

#define is_hex(c)	(\
		((c) >= '0' && (c) <='9') ||\
		((c) >= 'a' && (c) <='f') ||\
		((c) >= 'A' && (c) <='F'))
#define is_octal(c)		((c) >= '0' && (c) <='7')
#define is_decimal(c)	((c) >= '0' && (c) <='9')

#define to_lower(c)	(((c) >= 'A' && (c) <= 'Z') ? (c) - 'A' + 'a' : (c))
#define hex2d(c)	(\
	(c) >= '0' && (c) <= '9' ? (c) - '0' :\
	to_lower(c) >= 'a' && to_lower(c) <= 'f' ? to_lower(c) - 'a' + 10 :\
	-1)

#define FLAG_MINUS	(1<<0)  /* '-', left-align */
#define FLAG_PLUS	(1<<1)  /* '+', add a prefix for the signed-numeric type. Positive: '+', negative: '-' */
#define FLAG_SPACE	(1<<2)  /* ' ', add a prefix for the signed-numeric type. Positive: ' ', negative: '-' */
#define FLAG_ZERO	(1<<3)  /* '0', add '0' to fill width when 'width' is specified */
#define FLAG_HASH	(1<<4)  /* '#', 
                                   For g and G types, trailing zeros are not removed.
                                   For f, F, e, E, g, G types, the output always contains a decimal point.
                                   For o, x, X types, the text 0, 0x, 0X, respectively, is prepended to non-zero numbers. */

#define LENGTH_hh	(1<<0)  /* char-sized */
#define LENGTH_h	(1<<1)  /* short-sized */
#define LENGTH_l	(1<<2)  /* long-sized */
#define LENGTH_ll	(1<<3)  /* longlong-sized */
#define LENGTH_z	(1<<4)  /* size_t-sized */

static char *uintmax_to_text(uintmax_t a, int base, char *buff)
{
    char *htab = "0123456789abcdef";
    char *b = buff;
    char *e = buff;
    do {
        *e++ = htab[a % base];
        a /= base;
    }
    while (a);
    *e-- = '\0';
    for (; b < e; b++, e--) {
        char tmp = *e;
        *e = *b;
        *b = tmp;
    }
    return buff;
}

static void format_printc(int *counter, char **buff, int c)
{
    if (buff)
        *(*buff)++ = c;
    else
        putchar(c);

    if (counter)
        *counter += 1;
}

static void format_prints(int *counter, char **buff, const char *s)
{
    while (*s)
        format_printc(counter, buff, *s++);
}

static void
format_print_string(int *counter, char **buff, int flags,
                    int width, const char *s)
{
    int l = strlen(s);
    int p0 = width > l ? width - l : 0;

    /*right-align */
    if (!(flags & FLAG_MINUS))
        while (p0--)
            format_printc(counter, buff, ' ');

    format_prints(counter, buff, s);

    /*lift-align */
    if (flags & FLAG_MINUS)
        while (p0--)
            format_printc(counter, buff, ' ');
}

static void
format_print_number(int *counter, char **buff, int flags,
                    int width, int precision, int length,
                    char type, intmax_t number)
{
    int l = 0;
    char buff_i[64];
    int sign = 0;
    if (type == 'd' || type == 'i') {
        sign = (number < 0);
        number = sign ? -number : number;
    }
    if (flags & FLAG_PLUS)
        flags &= ~FLAG_SPACE;
    precision = precision ? precision : 1;

    switch (length) {
    case LENGTH_hh:
        if (type == 'i' || type == 'd' || type == 'u' || type == 'o'
            || type == 'x' || type == 'X')
            number = (unsigned char) (number);
        break;
    case LENGTH_h:
        if (type == 'i' || type == 'd' || type == 'u' || type == 'o'
            || type == 'x' || type == 'X')
            number = (unsigned short) (number);
        break;
    case LENGTH_l:
        if (type == 'i' || type == 'd' || type == 'u' || type == 'o'
            || type == 'x' || type == 'X')
            number = (unsigned long) (number);
        break;
    case LENGTH_ll:
        if (type == 'i' || type == 'd' || type == 'u' || type == 'o'
            || type == 'x' || type == 'X')
            number = (unsigned long long) (number);
        break;
    case LENGTH_z:
        if (type == 'i' || type == 'd' || type == 'u' || type == 'o'
            || type == 'x' || type == 'X')
            number = (size_t) (number);
        break;
    default:
        break;
    }

    if (type == 'o') {          /* octal */
        l += ((number != 0) && (flags & FLAG_HASH)) ? 1 : 0;    /* 0ooo */
        uintmax_to_text(number, 8, buff_i);
    } else if (type == 'x' || type == 'X') {    /* hex */
        l += ((number != 0) && (flags & FLAG_HASH)) ? 2 : 0;    /* 0xhhh */
        uintmax_to_text(number, 16, buff_i);
    } else {                    /* decimal */
        l += ((flags & (FLAG_PLUS | FLAG_SPACE)) || sign) ? 1 : 0;      /* +/-ddd */
        uintmax_to_text(number, 10, buff_i);
    }
    l += strlen(buff_i);

    int p0 = precision > l ? precision - l : 0;
    int p1 = width > MAX(l, precision) ? width - MAX(l, precision) : 0;

    if (flags & FLAG_MINUS) {
        /* left-align */
        if (type == 'd' || type == 'i') {       /* signed */
            if ((flags & (FLAG_PLUS | FLAG_SPACE)) || sign)
                format_printc(counter, buff,
                              sign ? '-' : (flags & FLAG_PLUS) ?
                              '+' : ' ');
        } else {                /* unsigned */
            if ((number != 0) && (flags & FLAG_HASH))
                format_prints(counter, buff,
                              type == 'x' ? "0x" :
                              type == 'X' ? "0X" : type == 'o' ? "0" : "");
        }

        /* precision fill */
        while (p0--)
            format_printc(counter, buff, '0');

        /* number */
        format_prints(counter, buff, buff_i);

        /* width fill */
        while (p1--)
            format_printc(counter, buff, ' ');
    } else {
        /* right-align */

        /* width fill */
        if (!(flags & FLAG_ZERO))
            while (p1--)
                format_printc(counter, buff, ' ');

        if (type == 'd' || type == 'i') {       /* signed */
            if ((flags & (FLAG_PLUS | FLAG_SPACE)) || sign) {
                format_printc(counter, buff,
                              sign ? '-' : (flags & FLAG_PLUS) ?
                              '+' : ' ');
            }
        } else {                /* unsigned */
            if ((number != 0) && (flags & FLAG_HASH)) {
                format_prints(counter, buff,
                              type == 'x' ? "0x" :
                              type == 'X' ? "0X" : type == 'o' ? "0" : "");
            }
        }

        /* width fill */
        if (flags & FLAG_ZERO)
            while (p1--)
                format_printc(counter, buff, '0');

        /* precision fill */
        while (p0--)
            format_printc(counter, buff, '0');

        /* number */
        format_prints(counter, buff, buff_i);
    }
}

static int format_print(char **buff, const char *format, va_list ap)
{
    int counter = 0;
    while (*format) {
        if (*format == '\\') {
            int c;
            format++;
            do {
                if (*format == 'a') {
                    c = '\a';
                    break;
                }
                if (*format == 'b') {
                    c = '\b';
                    break;
                }
                if (*format == 'f') {
                    c = '\f';
                    break;
                }
                if (*format == 'n') {
                    c = '\n';
                    break;
                }
                if (*format == 'r') {
                    c = '\r';
                    break;
                }
                if (*format == 't') {
                    c = '\t';
                    break;
                }
                if (*format == 'v') {
                    c = '\v';
                    break;
                }
                /* \xhh \Xhh */
                if (*format == 'x' || *format == 'X') {
                    format++;
                    c = hex2d(*format);
                    if (is_hex(format[1])) {
                        format++;
                        c = (c << 4) + hex2d(*format);
                    }
                    break;
                }
                if (is_octal(*format)) {
                    c = hex2d(*format);
                    if (is_octal(format[1])) {
                        format++;
                        c = (c << 3) + hex2d(*format);
                        if (is_octal(format[1])) {
                            format++;
                            c = (c << 3) + hex2d(*format);
                        }
                    }
                    break;
                }
                c = *format;    /* \', \", \\ */
            }
            while (0);
            format_printc(&counter, buff, c);
        } else if (*format == '%') {
            format++;
            do {
                if (*format == '%') {
                    format_printc(&counter, buff, '%');
                    break;
                }
                /* %[flags][width][.precision][length]type */
                int flags = 0;
                int width = 0;
                int precision = 0;
                int length = 0;
                char type;
                intmax_t number;
                /* flags */
                while (*format == '+' || *format == '-'
                       || *format == '#' || *format == '0'
                       || *format == ' ') {
                    if (*format == '+')
                        flags |= FLAG_PLUS;
                    if (*format == '-')
                        flags |= FLAG_MINUS;
                    if (*format == '#')
                        flags |= FLAG_HASH;
                    if (*format == '0')
                        flags |= FLAG_ZERO;
                    if (*format == ' ')
                        flags |= FLAG_SPACE;
                    format++;
                }
                /* width */
                if (*format == '*') {
                    format++;
                    width = va_arg(ap, int);
                } else {
                    while (is_decimal(*format)) {
                        width = width * 10 + hex2d(*format);
                        format++;
                    }
                }
                /* precision */
                if (*format == '.') {
                    format++;
                    if (*format == '*') {
                        format++;
                        precision = va_arg(ap, int);
                    } else {
                        while (is_decimal(*format)) {
                            precision = precision * 10 + hex2d(*format);
                            format++;
                        }
                    }
                }
                /* length */
                do {
                    if (*format == 'h') {
                        format++;
                        length = LENGTH_h;
                        if (*format == 'h') {
                            format++;
                            length = LENGTH_hh;
                        }
                        break;
                    }
                    if (*format == 'l') {
                        format++;
                        length = LENGTH_l;
                        if (*format == 'l') {
                            format++;
                            length = LENGTH_ll;
                        }
                        break;
                    }
                    if (*format == 't') {
                        format++;
                        length = LENGTH_z;
                        break;
                    }
                }
                while (0);
                /* type */
                do {
                    /* char */
                    if (*format == 'c') {
                        format++;
                        char s[2];
                        s[0] = va_arg(ap, int);
                        s[1] = 0;
                        format_print_string(&counter,
                                            buff, flags, width, s);
                        break;
                    }
                    /* pointer */
                    if (*format == 'p') {
                        format++;
                        type = 'x';
                        flags |= FLAG_HASH;
                        length = 0;
                        intmax_t number = va_arg(ap, uintptr_t);
                        format_print_number(&counter,
                                            buff, flags,
                                            width,
                                            precision, 0, 'x', number);
                        break;
                    }
                    /* number */
                    if (*format == 'd' || *format == 'i'
                        || *format == 'u' || *format == 'x'
                        || *format == 'X' || *format == 'o') {
                        intmax_t number;
                        type = *format++;
                        switch (length) {
                        case LENGTH_hh:
                        case LENGTH_h:
                            if (type == 'd' || type == 'i')
                                number = va_arg(ap, signed
                                                int);
                            else
                                number = va_arg(ap, unsigned
                                                int);
                            break;
                        case LENGTH_l:
                            if (type == 'd' || type == 'i')
                                number = va_arg(ap, signed
                                                long);
                            else
                                number = va_arg(ap, unsigned
                                                long);
                            break;
                        case LENGTH_ll:
                            if (type == 'd' || type == 'i')
                                number = va_arg(ap, signed
                                                long
                                                long);
                            else
                                number = va_arg(ap, unsigned
                                                long
                                                long);
                            break;
                        case LENGTH_z:
                            if (type == 'd' || type == 'i')
                                number = va_arg(ap, ssize_t);
                            else
                                number = va_arg(ap, size_t);
                            break;
                        default:
                            if (type == 'd' || type == 'i')
                                number = va_arg(ap, signed
                                                int);
                            else
                                number = va_arg(ap, unsigned
                                                int);
                            break;
                        }
                        format_print_number(&counter,
                                            buff, flags,
                                            width,
                                            precision,
                                            length, type, number);
                        break;
                    }
                    /* string */
                    if (*format == 's') {
                        format++;
                        format_print_string(&counter,
                                            buff, flags,
                                            width, va_arg(ap, char
                                                          *));
                        break;
                    }
                }
                while (0);
            }
            while (0);
        } else {
            format_printc(&counter, buff, *format);
            format++;
        }
    }

    if (buff)
        format_printc(NULL, buff, '\0');

    return counter;
}

int vsprintf(char *buff, const char *format, va_list ap)
{
    if (buff)
        return format_print(&buff, format, ap);
    return 0;
}

int vprintf(const char *format, va_list ap)
{
    return format_print(NULL, format, ap);
}

int sprintf(char *buff, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vsprintf(buff, format, ap);
    va_end(ap);
    return r;
}

int printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int r = vprintf(format, ap);
    va_end(ap);
    return r;
}
