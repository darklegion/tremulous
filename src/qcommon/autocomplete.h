#ifndef Q_AUTOCOMPLETE_H
#define Q_AUTOCOMPLETE_H

#include "qcommon/q_platform.h"

/*
==============================================================

Edit fields and command line history/completion

==============================================================
*/

#define	MAX_EDIT_LINE	256
struct field_t {
    int cursor;
    int scroll;
    int widthInChars;
    char buffer[MAX_EDIT_LINE];
};

void Field_Clear( field_t *edit );
void Field_AutoComplete( field_t *edit );
void Field_CompleteKeyname( void );
void Field_CompleteFilename( const char *dir, const char *ext, bool stripExt, bool allowNonPureFilesOnDisk );
void Field_CompleteCommand( char *cmd, bool doCommands, bool doCvars );
void Field_CompletePlayerName( const char **names, int count );
SO_PUBLIC void Field_CompleteList( char *listJson );

#endif
