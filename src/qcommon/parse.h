#ifndef Q_PARSE_H
#define Q_PARSE_H

#include "q_platform.h"

#define MAX_TOKENLENGTH		1024

#define TT_STRING					1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation

struct pc_token_t {
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
};

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

SO_PUBLIC bool Parse_AddGlobalDefine(char *string);
SO_PUBLIC int Parse_LoadSourceHandle(const char *filename);
SO_PUBLIC bool Parse_FreeSourceHandle(int handle);
SO_PUBLIC bool Parse_ReadTokenHandle(int handle, pc_token_t *pc_token);
SO_PUBLIC bool Parse_SourceFileAndLine(int handle, char *filename, int *line);

#endif
