//
// Testing Com_Parse()
//
#include "qcommon/q_shared.h"

#include <cstring>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

void Com_Error( int level, const char *error, ... ) { exit(0); }
void Com_Printf( const char *msg, ... ) {}

TEST_CASE("shader test")
{
    char* input = strdup(R"(
ui/assets/neutral/squad_h
{
  nopicmip
  {
    map ui/assets/neutral/squad_h.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen vertex
  }
})");
    char* save = input;

    char* token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "ui/assets/neutral/squad_h"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "{"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "nopicmip"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "{"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "map"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "ui/assets/neutral/squad_h.tga"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "blendFunc"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "GL_SRC_ALPHA"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "GL_ONE_MINUS_SRC_ALPHA"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "rgbgen"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "vertex"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "}"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "}"));

    free(save);
}

TEST_CASE("if statement")
{
    char* input = strdup(R"(if ( a ))");
    char* save = input;

    char* token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "if"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "("));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, "a"));

    token = COM_Parse(&input);
    REQUIRE(!strcmp(token, ")"));

    free(save);
}

