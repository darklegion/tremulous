#include "q3_lauxlib.h"

#include <sys/types.h>

#include <cstdarg>
#include <iostream>

#include "sys/sys_shared.h"

#include "cvar.h"
#include "msg.h"
#include "net.h"
#include "q_shared.h"
#include "qcommon.h"

size_t qlua_writestring(const char* string, size_t n)
{
#ifndef DEDICATED
    CL_ConsolePrint( string );
#endif
    Q_StripIndentMarker( const_cast<char*>(string) );
    Sys_Print( string );

    return n;
}

int qlua_writeline(void)
{
#ifndef DEDICATED
    CL_ConsolePrint( "\n" );
#endif
    Sys_Print( "\n" );
    return 0;
}

int qlua_writestringerror(const char *fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);
	char m[MAXPRINTMSG];
	Q_vsnprintf(m, sizeof(m), fmt, ap);
	va_end (ap);
    Com_Printf(S_COLOR_YELLOW "%s\n", m);
    return 0;
}

