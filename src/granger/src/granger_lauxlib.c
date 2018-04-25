#include <stdio.h>
#include <stdarg.h>

size_t lua_writestring(const char* string, size_t n)
{
    return fwrite(string, sizeof(char), n, stdout);
}

int lua_writeline(void)
{
    lua_writestring("\n", 1);
    fflush(stdout);
    return 0;
}

int lua_writestringerror(const char* fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fflush(stderr);
    return 0;
}

