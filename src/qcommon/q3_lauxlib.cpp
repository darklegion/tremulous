
#include <stdarg.h>
#include "q3_lauxlib.h"

#include <iostream>
#include <sys/types.h>

#include "cvar.h"
#include "q_shared.h"
#include "msg.h"
#include "net.h"
#include "qcommon.h"
#include "../sys/sys_shared.h"

size_t lua_writestring(char* string, size_t n)
{
#warning "FIXME: Tremded is not printing ANYTHING NOW"
#ifndef DEDICATED
    CL_ConsolePrint( string );
#endif
    Q_StripIndentMarker( string );
    Sys_Print( string );

    return n;
}

int lua_writeline(void)
{
#warning "FIXME: Tremded is not printing ANYTHING NOW"
#ifndef DEDICATED
    CL_ConsolePrint( "\n" );
#endif
    Sys_Print( "\n" );
    return 0;
}

int lua_writestringerror(const char *fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);
	char m[MAXPRINTMSG];
	Q_vsnprintf(m, sizeof(m), fmt, ap);
	va_end (ap);
    Com_Printf(S_COLOR_YELLOW "%s\n", m);
    return 0;
}

