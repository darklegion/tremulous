//
// Unit tests for CmdParser.h
//

#include "../CmdParser.h"

#include <iostream>
#include <vector>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("weird liner")
{
    Parser parser;
    auto args = parser.Parse(R"(/* */)");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "/*");
    REQUIRE(args[1] == "*/");
}

TEST_CASE("empty oneliner")
{
    Parser parser;
    auto args = parser.Parse(R"(//)");
    REQUIRE(args.size() == 0);
}

TEST_CASE("bind test 2")
{
    Parser parser;
    auto args = parser.Parse(R"(bind /)");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "bind");
    REQUIRE(args[1] == "/");

    args = parser.Parse(R"(bind \)");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "bind");
    REQUIRE(args[1] == "\\");
}

TEST_CASE("bind test")
{
    Parser parser;
    auto args = parser.Parse(R"(bind / +movedown)");
    REQUIRE(args.size() == 3);
    REQUIRE(args[0] == "bind");
    REQUIRE(args[1] == "/");
    REQUIRE(args[2] == "+movedown");
}

TEST_CASE("Cbuf_Execute formatted string 1")
{
    Parser parser;
    auto args = parser.Parse(R"(echo TEST \\\")");
    REQUIRE(args.size() == 3);
    REQUIRE(args[0] == "echo");
    REQUIRE(args[1] == "TEST");
    REQUIRE(args[2] == "\\\"");
}

TEST_CASE("reuse parser")
{
    Parser parser;
    auto args = parser.Parse("echo test");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "echo");
    REQUIRE(args[1] == "test");

    args = parser.Parse("test\n");
    REQUIRE(args.size() == 1);
    REQUIRE(args[0] == "test");
}

TEST_CASE("trailing whitespace")
{
    Parser parser;
    auto args = parser.Parse("set r_fullscreen 0 ");
    REQUIRE(args.size() == 3);
    REQUIRE(args[0] == "set");
    REQUIRE(args[1] == "r_fullscreen");
    REQUIRE(args[2] == "0");
}

TEST_CASE("echo Hello \"world\"")
{
    Parser parser;
    auto args = parser.Parse(R"(echo Hello "world")");
    REQUIRE(args.size() == 3);
    REQUIRE(args[0] == "echo");
    REQUIRE(args[1] == "Hello");
    REQUIRE(args[2] == "world");
}

TEST_CASE("quoted oneline comment")
{
    Parser parser;
    auto args = parser.Parse(R"("// A comment")");
    REQUIRE(args.size() == 1);
    REQUIRE(args[0] == "// A comment");
}

TEST_CASE("trailing oneline comment")
{
    Parser parser;
    auto args = parser.Parse(R"(This is a string// A comment)");
    REQUIRE(args[0] == "This");
    REQUIRE(args[1] == "is");
    REQUIRE(args[2] == "a");
    REQUIRE(args[3] == "string");
    REQUIRE(args.size() == 4);
}

TEST_CASE("oneline comment")
{
    Parser parser;
    auto args = parser.Parse(R"(// A comment)");
    REQUIRE(args.size() == 0);
}

TEST_CASE("escaped quotations")
{
    Parser parser;
    auto args = parser.Parse(R"("string \" string")");
    REQUIRE(args.size() == 1);
    REQUIRE(args[0] == "string \" string");
}

TEST_CASE("quoted string boundaries")
{
    Parser parser;
    auto args = parser.Parse(R"("A quoted string""another quoted string")");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "A quoted string");
    REQUIRE(args[1] == "another quoted string");
}

TEST_CASE("quoted string")
{
    Parser parser;
    auto args = parser.Parse(R"("A quoted string")");
    REQUIRE(args.size() == 1);
    REQUIRE(args[0] == "A quoted string");
}

TEST_CASE("quoted empty string")
{
    Parser parser;
    auto args = parser.Parse("\"\"");
    REQUIRE(args.size() == 1);
    REQUIRE(args[0] == "");
}

TEST_CASE("quoted empty string (tabs and whitespace)")
{
    Parser parser;
    auto args = parser.Parse("\" \t \"");
    REQUIRE(args.size() == 1);
    REQUIRE(args[0] == " \t ");
}

TEST_CASE("empty string (tab)")
{
    Parser parser;
    auto args = parser.Parse("\t");
    REQUIRE(args.size() == 0);
}

TEST_CASE("empty string (whitespace)")
{
    Parser parser;
    auto args = parser.Parse(" ");
    REQUIRE(args.size() == 0);
}

TEST_CASE("empty string")
{
    Parser parser;
    auto args = parser.Parse("");
    REQUIRE(args.size() == 0);
}

TEST_CASE(R"(tok1 "tok2 tok2")")
{
    Parser parser;
    auto args = parser.Parse(R"(tok1 "tok2 tok2")");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "tok1");
    REQUIRE(args[1] == "tok2 tok2");
}

TEST_CASE(R"(tok1 "tok2")")
{
    Parser parser;
    auto args = parser.Parse(R"(tok1 "tok2")");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "tok1");
    REQUIRE(args[1] == "tok2");
}

TEST_CASE("tok1 tok2")
{
    Parser parser;
    auto args = parser.Parse("tok1 tok2");
    REQUIRE(args.size() == 2);
    REQUIRE(args[0] == "tok1");
    REQUIRE(args[1] == "tok2");
}
