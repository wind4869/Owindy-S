#include "type.h"
#include "util.h"

PUBLIC int vsprintf(char *buf, const char *fmt, va_list args)
{
    va_list vargs = args;
    
    int num = 0;
    char chr, *str, tmp[20];
    long lint;

    char *p_buf = buf;

    struct attributes {
        u32_t size_long : 1;
    } attrs;

    while ((chr = *fmt++)) {
        if (chr == '%') {
            memset(&attrs, 0, sizeof (attrs));

next_attr_type:
            switch ((chr = *fmt++)) {
            case 'l':
                attrs.size_long = true;
                goto next_attr_type;

            case '%':
                *p_buf++ = '%';
                num++;
                break;

            case 's':
                str = va_arg(vargs, char*);
puts:
                while (*str) {
                    *p_buf++ = *str++;
                    num++;
                }
                break;
                
            case 'd':
                lint = va_arg(vargs, long);
                if (!attrs.size_long)
                    lint = (int)lint;
                if (lint < 0) {
                    *p_buf++ = '-';
                    lint = -lint;
                    num++;
                }
                str = ultoa(lint, tmp, 10);
                goto puts;

            case 'x':
            case 'X':
                lint = va_arg(vargs, long);
                if (!attrs.size_long)
                    lint = (unsigned int)lint;
                str = ultoa(lint, tmp, chr == 'x' ? 16 : -16);
                goto puts;

            case 'c':
                tmp[0] = (char)va_arg(vargs, int), tmp[1] = 0, str = tmp;
                goto puts;
            }
        } else {
            *p_buf++ = chr;
            num++;
        }
    }

    return num;
}

PUBLIC int sprintf(char *buf, const char *fmt, ...)
{
    int i;
    va_list args;
    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    va_end(args);
    return i;
}

