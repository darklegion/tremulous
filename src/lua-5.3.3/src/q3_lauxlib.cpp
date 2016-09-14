#include "q3_lauxlib.h"

#include "../../qcommon/qcommon.h"
#include "../../qcommon/q_shared.h"

size_t lua_writestring(char* string, size_t n)
{
    if ( n > MAXPRINTMSG ) {
         n = MAXPRINTMSG;
        string[n] = '\0';
    }
#ifndef DEDICATED
    CL_ConsolePrint( string );
#endif
    Q_StripIndentMarker( string );
    Sys_Print( string );

    return n;
}

int lua_writeline(void)
{
#ifndef DEDICATED
    CL_ConsolePrint( "\n" );
#endif
    Q_StripIndentMarker( "\n" );
    Sys_Print( "\n" );
    return 0;
}

int lua_writestringerror(const char *s, p)
{
    std::string fmt;
    fmt = S_COLOR_YELLOW;
    fmt += s;
    Com_Printf(fmt.c_str(), (p));
    return 0;
}

