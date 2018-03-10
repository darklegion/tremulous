// Unit tests for MSG_ReadString, MSG_ReadBigString and MSG_ReadStringLine
// Victor Roemer (wtfbbqhax), <victor@badsec.org>.
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#define PARANOID
#include "qcommon/cvar.h"
#include "qcommon/huffman.h"
#include "qcommon/msg.h"
#include "qcommon/net.h"

using namespace std;

cvar_t *cl_shownet = nullptr;

void Com_Error( int code, const char *fmt, ... ) { abort(); }
void Com_Printf( const char *fmt, ... ) { }

TEST_CASE("MSG_ReadString consumes the trailing '\\0' (OOB)")
{
    byte msg_buf[MAX_STRING_CHARS + 1];
    msg_t msg;

    char str[MAX_STRING_CHARS];
    memset(&str, 'A', sizeof(str));
    str[sizeof(str)-1] = '\0';

    MSG_InitOOB(&msg, msg_buf, sizeof(msg_buf));
    MSG_WriteString2(&msg, str, sizeof(msg_buf));
    MSG_WriteByte(&msg, 55);

    MSG_BeginReadingOOB(&msg);
    MSG_ReadString(&msg);
    REQUIRE(MSG_ReadByte(&msg) == 55);
}

TEST_CASE("MSG_ReadBigString consumes the trailing '\\0' (OOB)")
{
    byte msg_buf[BIG_INFO_STRING + 1];
    msg_t msg;

    char str[BIG_INFO_STRING];
    memset(&str, 'A', sizeof(str));
    str[sizeof(str)-1] = '\0';

    MSG_InitOOB(&msg, msg_buf, sizeof(msg_buf));
    MSG_WriteString2(&msg, str, sizeof(msg_buf));
    MSG_WriteByte(&msg, 55);

    MSG_BeginReadingOOB(&msg);
    MSG_ReadBigString(&msg);
    REQUIRE(MSG_ReadByte(&msg) == 55);
}

TEST_CASE("MSG_ReadStringLine consumes the trailing '\\0' (OOB)")
{
    byte msg_buf[MAX_STRING_CHARS + 1];
    msg_t msg;

    char str[MAX_STRING_CHARS];
    memset(&str, 'A', sizeof(str));
    str[sizeof(str)-1] = '\0';

    MSG_InitOOB(&msg, msg_buf, sizeof(msg_buf));
    MSG_WriteString2(&msg, str, sizeof(msg_buf));
    MSG_WriteByte(&msg, 55);

    MSG_BeginReadingOOB(&msg);
    MSG_ReadStringLine(&msg);
    REQUIRE(MSG_ReadByte(&msg) == 55);
}

TEST_CASE("full message - MSG_ReadString")
{
    byte msg_buf[MAX_STRING_CHARS * 2];
    msg_t msg;

    char str[sizeof(msg_buf)];
    memset(&str, 'A', sizeof(str));
    str[sizeof(str)-1] = '\0';

    MSG_Init(&msg, msg_buf, sizeof(msg_buf));
    MSG_WriteString2(&msg, str, sizeof(msg_buf));

    MSG_BeginReading(&msg);
    const char* ret = MSG_ReadString(&msg);

    REQUIRE(strlen(ret) == 1023);
}

TEST_CASE("MSG_ReadStringLine (OOB)")
{
    byte msg_buf[MAX_MSGLEN];
    msg_t msg;
    
    MSG_InitOOB(&msg, msg_buf, sizeof(msg_buf));
    MSG_WriteString(&msg, "Hello world!\nI am a message!");

    MSG_BeginReadingOOB(&msg);
    string ret { MSG_ReadStringLine(&msg) };

    REQUIRE(ret == "Hello world!");

    ret = MSG_ReadStringLine(&msg);
    REQUIRE(ret == "I am a message!");

    int c = MSG_ReadChar(&msg);
    REQUIRE(c == -1);

    // Can't read past the EOL
    ret = MSG_ReadStringLine(&msg);
    REQUIRE(ret == "");
}

TEST_CASE("MSG_ReadStringLine")
{
    byte msg_buf[MAX_MSGLEN];
    msg_t msg;
    
    MSG_Init(&msg, msg_buf, sizeof(msg_buf));
    MSG_WriteString(&msg, "Hello world!\nI am a message!");

    MSG_BeginReading(&msg);
    string ret { MSG_ReadStringLine(&msg) };

    REQUIRE(ret == "Hello world!");

    ret = MSG_ReadStringLine(&msg);
    REQUIRE(ret == "I am a message!");

    // Can't read past the EOL
    ret = MSG_ReadStringLine(&msg);
    REQUIRE(ret == "");
}
