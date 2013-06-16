#include "type.h"
#include "util.h"
#include "tty.h"
#include "kernel.h"

int printf(const char *fmt, ...)
{
    int i;
    char buf[512];

    va_list args;

    va_start(args, fmt);
    i = vsprintf(buf, fmt, args);
    tty_write(&tty0, buf, i);
    va_end(args);

    return i;
}

